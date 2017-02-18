#ifndef __my_socket__
#define __my_socket__
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cassert>
#include<algorithm>
#ifdef _WIN32
    #include "winsock2.h"
    typedef int socklen_t;
void ShowErrorMsg(){
    wchar_t hlocal[256]; hlocal[0] = 0;
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        (LPSTR)&hlocal, 255, nullptr);
    fputws(hlocal,stderr);
}
#else
    typedef int SOCKET;
    constexpr int INVALID_SOCKET = -1;
    constexpr int SOCKET_ERROR = -1;
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <errno.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef sockaddr_in SOCKADDR_IN;
#endif
bool init_socket(){
#ifdef _WIN32
    WSADATA wsaData;
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) != 0 ){
        puts("Socket WSAStartup() failed.");
        ShowErrorMsg();
        return false;
    }
#endif
    return true;
}
class skt{
    SOCKET fd;
    bool InitSocket(const char *ip, int port);
public:
    skt(SOCKET f_):fd(f_){
        init_socket();
    }
    skt(skt&&b):fd(b.fd){
        b.fd = INVALID_SOCKET;
        init_socket();
    }
    skt(const char *ip,int port){
        init_socket();
        auto t(8);
        for(;t && !InitSocket(ip,port);--t);
        puts("InitSocket fail");
        fflush(stdout);
        assert(t && "InitSocket fail");
    }
    skt&operator=(skt&&b){
        fd = b.fd; b.fd = INVALID_SOCKET;
        return *this;
    }
    skt(const skt&) = delete;
    skt&operator=(const skt&) = delete;
    ~skt(){
    #ifdef _WIN32
        if(fd!=INVALID_SOCKET)closesocket(fd);
        WSACleanup();
    #else
        if(fd!=INVALID_SOCKET)close(fd);
    #endif
    }
    char*Recv_t(char*buf,size_t len,timeval tv)const{
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        const auto res = select(fd+1, &fds, nullptr, nullptr, &tv);
        if(res <= 0){
            printf("recv error: %d\n",res);
            return buf;
        }
        return Recv_r(buf,len);
    }
    template<bool silent=false>
    char*Recv_r(char*buf,size_t len)const{
        const char*s(buf);
        buf+= recv(fd, buf, len, 0); *buf = 0;
        if(!silent)printf("recv:%s\n",s);
        return buf;
    }
    template<bool silent=false>
    void Send_r(const char*s)const{
        if(!silent)printf("send:%s\n",s);
        send(fd, s, strlen(s),0);
    }
    template<bool silent=false>
    void Send_r(const char*s,size_t len)const{
        if(!silent)printf("send:%s\n",s);
        send(fd, s, len,0);
    }
};
bool skt::InitSocket(const char *ip, int port){
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( fd == INVALID_SOCKET ){
        puts("Socket Initial failed.");
    #ifdef _WIN32
        ShowErrorMsg();
    #endif
        return false;
    }
    SOCKADDR_IN servAddr; memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = inet_addr(ip);
    // connect to Server
    if( connect(fd, (sockaddr*)&servAddr, sizeof(servAddr) ) == SOCKET_ERROR ){
    #ifdef _WIN32
        puts("//Connect Error.  please call the  Administrator.//");
        closesocket(fd);
    #else
        printf("connect failed. Error: %s\n",strerror(errno));
        close(fd);
    #endif
        return false;
    }
    printf("Connected at %s:%d\n",ip,port);
    return true;
}
class get_sfd{
    SOCKET sfd;
    sockaddr_in address;
public:
    get_sfd(int port){
        if(init_socket() == 0){
            exit(1);
        }
        sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sfd == INVALID_SOCKET){
        #ifdef _WIN32
            fputs("Socket Initial failed.\n", stderr);
            WSACleanup();
            ShowErrorMsg();
        #else
            perror("server: create");
        #endif
            exit(1);
        }
        memset(&address,0,sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if(bind(sfd, (sockaddr*) &address, sizeof(address)) == 0){
            printf("Binding Socket\n"); fflush(stdout);
        }else{
            perror("server: bind"); exit(1);
        }
        if(listen(sfd, 10) >= 0){
            printf("listening...\n"); fflush(stdout);
        }else{
            perror("server: listen"); exit(1);
        }
    }
    SOCKET get_cfd_t(timeval tv)const{
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sfd, &fds);
        const auto res = select(sfd+1, &fds, nullptr, nullptr, &tv);
        if(res <= 0){
            printf("get_cfd error: %d\n",res);
            return INVALID_SOCKET;
        }
        return get_cfd();
    }
    SOCKET get_cfd()const{
        socklen_t addrlen(sizeof(address));
        const auto cfd = accept(sfd, (sockaddr*) &address, &addrlen);
        if(cfd == INVALID_SOCKET){
            perror("server: accept"); exit(1);
        }else{
            printf("The Client is connected at fd %d\n",int(cfd));
            fflush(stdout);
        }
        return cfd;
    }
    ~get_sfd(){
    #ifdef _WIN32
         closesocket(sfd);
         WSACleanup();
    #else
         close(sfd);
    #endif
    }
};
#endif
