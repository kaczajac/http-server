#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>

#include <core_types.h>
#include <core_allocator.h>
#include <core_string.h>
#include <core_threadpool.h>

#include <http_parsers.h>
#include <http_request.h>
#include <http_response.h>
#include <http_headers.h>
#include <http_server.h>

static volatile sig_atomic_t __http_post_exit;

static void http_interrupt_handler(i32 signum) {
    (void) signum;
    __http_post_exit = TRUE;
}

static void set_default_headers(struct HttpResponse *response) {

    struct HttpHeader default_headers[] = {
        {   .key = str_fromlit("Connection"),   .value = str_fromlit("close")   }
    };

    response->header_count = (u32) (sizeof(default_headers) / sizeof(default_headers[0]));

    for (u8 header_id = 0; header_id < response->header_count; header_id++) {
        response->headers[header_id].key     = default_headers[header_id].key;
        response->headers[header_id].value   = default_headers[header_id].value;
    }

}

static void *http_server_routine(void *arg) {
    
    struct HttpServer *server       = (struct HttpServer *) arg;
    struct sockaddr_in client_addr  = { 0 };
    socklen_t client_addrlen        = sizeof(client_addr);

    /*
     * Assume only index.html
     * can be served for now
     */

    const u8 *webpage_path  = (u8 *) "index.html";
    string webpage          = str_fromfile((char *) webpage_path, server->context);

    if (webpage.size > 0) {
        printf("[INFO] Listening on %s at %d\n", server->listener->ip_lit, server->listener->port);
        while (server->running) {
        
            i64 client_fd = accept(server->listener->fd, (struct sockaddr *) &client_addr, &client_addrlen);
            
            if (client_fd != -1) {

                u8 client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(client_addr.sin_addr), (char *) client_ip, INET_ADDRSTRLEN);

                printf("[INFO] Accepted a new request from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
                struct MemoryBlock *block = memorypool_reserveblock();

                if (block != NULL) {
                    struct ConnectionArgs *connection_args = memblock_alloc(block, sizeof(struct ConnectionArgs));
                    connection_args->client_fd        = client_fd;
                    connection_args->client_addr      = client_addr;
                    connection_args->client_addrlen   = client_addrlen;
                    connection_args->webpage          = webpage;
                    threadpool_execute(server->pool, (JobFunction) server->conn_handler, (void *) block);
                } else {
                    struct HttpResponse response = { .status = STATUS_503 };
                    set_default_headers(&response);
                    httpresponse_send(&response, client_fd);
                    close(client_fd);
                }

            }

            if (__http_post_exit) {
                server->running = FALSE;
            }

        }

    } else {
        fprintf(stderr, "[ERROR] Failed to find index.html in current working directory\n");
    }

    printf("[INFO] Waiting for all connections to be handled...\n");
    http_server_destroy(server);
    printf("[INFO] Done\n");
    return NULL;

}

static void *http_connection_handler(void *arg) {

    struct MemoryBlock *block = arg;
    struct ConnectionArgs *conn_args = (struct ConnectionArgs *) block->data; 
    const string server_target = str_fromlit("/");
    
    /*
     * Try to read and parse the request
     * from client connection and set the
     * response status
     */ 

    struct HttpRequest request = {0};
    struct HttpResponse response = {0};
    
    boolean good_request = http_parse_request(conn_args->client_fd, &request);
    if (!good_request) {
        response.status = STATUS_400;
        goto send_response;
    }

    boolean target_found = str_equals(request.request_line.target, server_target);
    if (!target_found) {
        response.status = STATUS_404;
        goto send_response;
    }

    response.status = STATUS_200;

send_response:

    /*
     * Build and send the response
     * back to the client
     */

    set_default_headers(&response);
   
    if (response.status == STATUS_200) {
        u8 length[100];
        memset(length, 0, sizeof(length));
        snprintf((char *) length, sizeof(length) - 1, "%ld", conn_args->webpage.size);
        httpheaders_set(
            response.headers, 
            &response.header_count, 
            str_fromlit("Content-Length"), 
            str_fromlit(length) 
        );
        httpheaders_set(
            response.headers, 
            &response.header_count, 
            str_fromlit("Content-Type"), 
            str_fromlit("text/html")
        );
        response.body = conn_args->webpage;
    }

    httpresponse_send(&response, conn_args->client_fd);

    shutdown(conn_args->client_fd, SHUT_RDWR);
    close(conn_args->client_fd);
    memorypool_returnblock(block);
    return NULL;

}

int main(const int argc, const char **argv) {

    (void) argc;
    (void) argv;

    struct HttpServer *server = http_server_create(
        http_connection_handler,
        http_interrupt_handler,
        http_server_routine
    );

    if (server == NULL) {
        printf("[ERROR] Failed to create the server. Please try again in a few seconds\n");
        return -1;
    }

    pthread_t server_joinhandle;
    if (http_server_run(server, &server_joinhandle) != 0) {
        printf("[ERROR] Failed to start the server thread\n");
        return -1;
    }

    pthread_join(server_joinhandle, 0);
    return 0;

}
