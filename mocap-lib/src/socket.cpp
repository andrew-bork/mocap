#include <socket.h>
#include <cstdio>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

inline int _socket(int d, int t, int p){
    return socket(d, t, p);
}

sock::socket::socket(sock_domain domain, sock_type type){
    fd = _socket((int) domain, (int) type, 0);
    opened = false;
    addr = 0;
    port = 0;
}



inline int _bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    return bind(sockfd, addr, addrlen);
}

inline int _listen(int sockfd, int back){
    return listen(sockfd, back);
}

inline int _accept(int fd, sockaddr *addr, socklen_t *addrlen){
    return accept(fd, addr, addrlen);
}


int sock::socket::setSocketOption(int lvl, int opt, const void * optval, socklen_t optlen){
    return setsockopt(fd,lvl, opt, &optval, sizeof(optval));
}

int sock::socket::inBind(int addr, int port){
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = addr;
    address.sin_port = htons(port);
    int len = sizeof(address);
    int e = bind(fd, (sockaddr *) &address, len);
    // if(e<0){
    //     printf("Failed to bind.\n");
    // }
    return e;
}

int sock::socket::unixBind(const char* path){
    sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path);
    int len = sizeof(address);
    int e = bind(fd, (sockaddr *) &address, len);
    // if(e<0){
    //     perror("Failed to bind.\n");
    // }
    return e;
}

int sock::socket::listen(int backlog){
    int e = _listen(fd, backlog);
    // if(e<0){
    //     perror("Failed to listen.\n");
    // }
    return e;
}

sock::un_connection sock::socket::un_accept(){
    un_connection c;
    c.s = this;
    int len = sizeof(c.addr);
    c.fd = _accept(fd, (sockaddr *)&c.addr, (socklen_t *) &len);
    c.valid = c.fd>=0;
    // if(!c.valid){
    //     perror("Failed to accept.\n");
    // }
    return c;
}

sock::in_connection sock::socket::in_accept(){
    in_connection c;
    c.s = this;
    int len = sizeof(c.addr);
    c.fd = _accept(fd, (sockaddr *)&c.addr, (socklen_t *) &len);
    c.valid = c.fd>=0;
    // if(!c.valid){
    //     perror("Failed to accept.\n");
    // }
    return c;
}

sock::in_connection sock::socket::in_connect(int addr, int port){
    in_connection c;
    c.s = this;
    c.addr.sin_family = AF_INET;
    c.addr.sin_addr.s_addr = addr;
    c.addr.sin_port = htons(port);
    c.s = this;
    int len = sizeof(c.addr);
    c.valid = connect(fd, (sockaddr *) &c.addr, len) >= 0;
    c.fd = fd;
    // if(!c.valid){
    //     perror("Failed to connect.\n");
    // }
    return c;

}

sock::un_connection sock::socket::un_connect(const char * path){
    un_connection c;
    c.addr.sun_family = AF_UNIX;
    strcpy(c.addr.sun_path, path);
    int len = sizeof(c.addr);
    c.valid = connect(fd, (sockaddr *) &c.addr, len) >= 0;
    c.fd = fd;
    // if(!c.valid){
    //     perror("Failed to connect.\n");
    // }
    return c;
}



inline int _read(int fd, char * n, int l){
    return read(fd, n, l);
}


inline int _send(int fd, const char * n, int l, int f){
    return send(fd, n, l, f);
}

int sock::un_connection::read(char * buffer, int len){
    int e = _read(fd, buffer, len);
    // if(e<0){
    //     perror("Error reading!\n");
    // }
    return e;
}
int sock::un_connection::send(const char * buffer, int len){
    int e = _send(fd, buffer, len, 0);
    // if(e<0){
    //     perror("Error sending!\n");
    // }
    return e;
}
int sock::in_connection::read(char * buffer, int len){
    int e = _read(fd, buffer, len);
    // if(e<0){
    //     perror("Error reading!\n");
    // }
    return e;
}
int sock::in_connection::send(const char * buffer, int len){
    int e = _send(fd, buffer, len, 0);
    // if(e<0){
    //     perror("Error sending!\n");
    // }
    return e;
}

void sock::socket::make_non_block(){
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

bool sock::un_connection::can_read(){
    pollfd fds;
    fds.fd = fd;
    fds.events = POLLIN;
    fds.revents = 0;
    return poll(&fds, 1, 0);
}

int _close(int fd){
    return close(fd);
}

int sock::un_connection::close(){
    valid = false;
    return _close(fd);
}

int sock::in_connection::close(){
    valid = false;
    return _close(fd);
}

int sock::socket::close(){
    opened = false;
    return _close(fd);
}