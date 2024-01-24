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
    bool START = false; //��־��ʼ�����ļ�
    bool END = false; //��־�ļ��������
    bool ACK = false; 
    u_short seq;//���к�
    u_short ack;//ȷ�Ϻ�
    u_long len;//���ݳ���
    u_long num; //���͵���Ϣ����������
    u_short checksum;//У���
    char data[datasize];//���ݳ���
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
u_short cksum(u_short* mes, int size); //����У���
void create_ClientSocket(SOCKET& Client); //�����ͻ����׽���
void create_ServerSocket(SOCKET& Server); //����������׽���