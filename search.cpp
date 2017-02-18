#include"OTP.h"
#include"my_socket.h"
#include<cstdio>
void mode0(){
    OTP EG;
    char ibuf[1024],obuf[1024];
    for(bool is_continued = true;is_continued;){
       char*st(ibuf);
       for(;st!=ibuf+1023 && (*st=getchar())!='\n';++st);
       *st = 0;
       is_continued = EG.do_op(ibuf,obuf,stderr);
       puts(obuf);
    }
}
void mode1(const char*ip,int port){
    const skt m_csock(ip,port);
    OTP EG;
    char ibuf[1024],obuf[1024];
    for(bool is_continued = true;is_continued;){
        const bool get_somthing = m_csock.Recv_r(ibuf,1023)!=ibuf;
        is_continued = get_somthing && EG.do_op(ibuf,obuf,stdout);
        m_csock.Send_r(obuf);
    }
}
int main(int argc, char*argv[]){
    switch(argc){
        case 1: mode0(); break;
        case 2: mode1("127.0.0.1",atoi(argv[1])); break;
        case 3: mode1(argv[1],atoi(argv[2])); break;
        default: puts("unknown command\n");
    }
    return 0;
}

