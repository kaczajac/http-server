#ifndef __LISTENER_H__
#define __LISTENER_H__


#include <unistd.h>
#include <netinet/in.h>

#include <core_types.h>
#include <core_allocator.h>


struct TcpListener {
    struct sockaddr_in  addr;
    i64                 fd;
    i64                 backlog;
    u16                 port;
    u8                  *ip_lit;
};


extern struct TcpListener *tcplistener_create(struct MemoryBlock *block, const u8 *ip_lit, u16 port, i32 backlog);
#define tcplistener_destroy(listener) close((listener)->fd);


#endif /* __LISTENER_H__ */
