#include <http_server.h>

#include <signal.h>
#include <pthread.h>


struct HttpServer *http_server_create(HttpConnectionHandler conn_handler, HttpInterruptHandler int_handler, HttpServerRoutine routine) {

    if (conn_handler == NULL || int_handler == NULL || routine == NULL)
        return NULL;

    struct MemoryBlock *server_context = memorypool_reserveblock();
    struct HttpServer *server = memblock_alloc(server_context, sizeof(struct HttpServer));
    server->context = server_context;

    server->listener = tcplistener_create(server->context, (const u8 *) "127.0.0.1", 6060, 10);
    if (server->listener == NULL) {
        memorypool_returnblock(server_context);
        return NULL;
    }

    server->pool = threadpool_create(server->context, MEMPOOL_BLKCNT);
    if (server->pool == NULL) {
        tcplistener_destroy(server->listener);
        memorypool_returnblock(server_context);
        return NULL;
    }

    server->conn_handler    = conn_handler;
    server->int_handler     = int_handler;
    server->routine         = routine;
    server->running         = FALSE;

    return server;

}


i64 http_server_run(struct HttpServer *server, pthread_t *joinhandle) {

    struct sigaction action     = { 0 };
    action.sa_handler           = (__sighandler_t) server->int_handler;
    i64 status                  = 0;

    status = sigaction(SIGINT, &action, NULL);
    if (status != 0) {
        return -1;
    }

    status = pthread_create(joinhandle, NULL, server->routine, (void *) server);
    if (status != 0) {
        return -1;
    }
    
    server->running = TRUE;
    return status;
    
}


void http_server_destroy(struct HttpServer *server) {

    threadpool_destroy(server->pool);           
    tcplistener_destroy(server->listener);
    memorypool_returnblock(server->context);

}
