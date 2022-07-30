#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>


namespace sock{
    enum sock_domain {
        unix = AF_UNIX, inet=AF_INET
    };
    enum sock_type {
        tcp=SOCK_STREAM, udp=SOCK_DGRAM
    };
    enum sock_opt {
        reuse_addr = SO_REUSEADDR,
        reuse_port = SO_REUSEPORT
    };
    struct socket;
    struct un_connection{
        socket * s;
        bool valid;
        sockaddr_un addr;
        int fd;
        int read(char * buffer, int len);
        bool can_read();
        int send(const char * buffer, int len);
        int close();
    };
    struct in_connection{
        socket * s;
        bool valid;
        sockaddr_in addr;
        int fd;
        int read(char * buffer, int len);
        bool can_read();
        int send(const char * buffer, int len);
        int close();
    };
    struct socket{
        int fd;
        int addr;
        int port;
        bool opened;
        //sock_domain::sock_domain domain;

        socket(sock_domain domain, sock_type type);

        int setSocketOption(int lvl, int opt, const void * optval, socklen_t optlen);
        int inBind(int addr, int port);
        int unixBind(const char* path);
        int listen(int backlog);
        
        void make_non_block();


        un_connection un_accept();
        in_connection in_accept();
    
        un_connection un_connect(const char* path);
        in_connection in_connect(int addr, int port);

        int close();
    };
}
#endif