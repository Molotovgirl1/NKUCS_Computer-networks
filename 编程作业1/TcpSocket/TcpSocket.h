#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include<WinSock2.h> //windowsƽ̨�������ͷ�ļ�
#pragma comment(lib,"ws2_32.lib") //�����
#include<pthread.h> //���߳�ͷ�ļ�
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
//��������̵߳Ľṹ��
struct serv_thread {
	char *name;
	SOCKADDR_IN sockaddr;
	SOCKET clientfd;
	pthread_t thread;
};
//�ͻ������̵߳Ľṹ��
struct client_thread
{
	SOCKET clientfd;
	char* name;
};
extern vector<SOCKET>All_Clientfd;
extern pthread_t send_thread;
//�������
bool open_Socket();
//�ر����翨
bool close_Socket();
//����������socket
SOCKET create_ServerSocket();
//�����ͻ���socket
SOCKET create_ClientSocket();
//�������ͻ��˷�����Ϣ
void* serv_send(void* arg);
//����˽��տͻ�����Ϣ
void* serv_recv(void* arg);
//�ͻ��������˷�����Ϣ
void* client_send(void* arg);
//�ͻ��˽��շ������Ϣ
void* client_recv(void* arg);
void broadcast(char * name,char * recbuf);
#endif // !TCPSOCKET_H
