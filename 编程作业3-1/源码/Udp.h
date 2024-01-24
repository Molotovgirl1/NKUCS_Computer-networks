#include <WinSock2.h>
#include<iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include<random>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#define SERVER_IP "127.0.0.1"
#define CLIENT_IP "127.0.0.1"
#define SERVER_PORT 8000
#define CLIENT_PORT 8888
#define BUFFER sizeof(message)
#define MAXTIME 200
#define datasize 1024
#define filepathlen 20
#define disconnecttime 3000
struct message
{
    bool SYN = false; 
    bool FIN = false;
    bool START = false; //标志开始传输文件
    bool END = false; //标志文件传输完成
    bool ACK = false; 
    u_short seq;//序列号
    u_short ack;//确认号
    u_long len;//数据长度
    u_long num; //发送的消息包含几个包
    u_short checksum;//校验和
    char data[datasize];//数据长度
    message() {
         SYN = false;
         FIN = false;
         START = false;
         END = false;
         ACK = false;
        seq = 0;
        ack = 0;
        len = 0;
        num = 0;
        checksum = 0;
        memset(data, 0, datasize);
    }
};
extern SOCKADDR_IN serveraddr, clientaddr;
extern int len;
void initial();
int getrand();
u_short cksum(u_short* mes, int size); //计算校验和
void create_ClientSocket(SOCKET& Client); //创建客户端套接字
void create_ServerSocket(SOCKET& Server); //创建服务端套接字