#include <http_tcplistener.h>

#include <sys/socket.h>
#include <arpa/inet.h>


struct TcpListener *tcplistener_create(struct MemoryBlock *block, const u8 *ip_lit, u16 port, i32 backlog) {

    struct TcpListener *listener = memblock_alloc(block, sizeof(struct TcpListener));
    if (listener == NULL) {
        return NULL;
    }

    listener->ip_lit            = (u8 *) ip_lit;
    listener->port              = port;
    listener->backlog           = backlog;
    listener->addr.sin_family   = AF_INET;
    listener->addr.sin_port     = htons(listener->port);
    inet_aton((const char *)listener->ip_lit, &listener->addr.sin_addr);
    
    if ((listener->fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) {
        return NULL;
    }

    if (bind(listener->fd, (struct sockaddr *) &listener->addr, sizeof(listener->addr)) == -1) {
        close(listener->fd);
        return NULL;
    }

    if (listen(listener->fd, listener->backlog) == -1) {
        close(listener->fd);
        return NULL;
    }

    return listener;

}
