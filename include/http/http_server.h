#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__


#include <netinet/in.h>

#include <core_types.h>
#include <core_threadpool.h>
#include <core_string.h>

#include <http_tcplistener.h>


typedef void* (*HttpConnectionHandler)(void *);
typedef void* (*HttpServerRoutine)(void *);
typedef void (*HttpInterruptHandler)(int);


struct ConnectionArgs {
    struct MemoryBlock  *block;
    struct sockaddr_in  client_addr;
    i32                 client_fd;
    socklen_t           client_addrlen;
    string              webpage;
};


struct HttpServer {
    struct MemoryBlock      *context;
    struct TcpListener      *listener;
    struct ThreadPool       *pool;
    HttpConnectionHandler   conn_handler;
    HttpInterruptHandler    int_handler;
    HttpServerRoutine       routine;
    boolean                 running;
};


extern struct HttpServer *http_server_create(HttpConnectionHandler conn_handler, HttpInterruptHandler int_handler, HttpServerRoutine routine);
extern i64 http_server_run(struct HttpServer *server, pthread_t *joinhandle);
extern void http_server_destroy(struct HttpServer *server);


#endif /*__HTTP_SERVER_H__ */
