#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include<WinSock2.h> //windows平台的网络库头文件
#pragma comment(lib,"ws2_32.lib") //网络库
#include<pthread.h> //多线程头文件
#pragma comment(lib, "pthreadVC2.lib")
#include<iostream>
#include<vector>
#define SERPORT 8888
#define IP "127.0.0.1"
# define MAX_REC_LEN 500
# define MAX_SEND_LEN 500
# define MAX_NAME_LEN 20
# define MAX_CONTENT_LEN MAX_REC_LEN+MAX_NAME_LEN+1
# define ENDSIGNAL "quit"
using namespace std;
//服务端子线程的结构体
struct serv_thread {
	char *name;
	SOCKADDR_IN sockaddr;
	SOCKET clientfd;
	pthread_t thread;
};
//客户端子线程的结构体
struct client_thread
{
	SOCKET clientfd;
	char* name;
};
extern vector<SOCKET>All_Clientfd;
extern pthread_t send_thread;
//打开网络库
bool open_Socket();
//关闭网络卡
bool close_Socket();
//创建服务器socket
SOCKET create_ServerSocket();
//创建客户端socket
SOCKET create_ClientSocket();
//服务端向客户端发送消息
void* serv_send(void* arg);
//服务端接收客户端消息
void* serv_recv(void* arg);
//客户端向服务端发送消息
void* client_send(void* arg);
//客户端接收服务端消息
void* client_recv(void* arg);
void broadcast(char * name,char * recbuf);
#endif // !TCPSOCKET_H
