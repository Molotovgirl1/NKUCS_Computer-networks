#include"./Udp.h"
SOCKADDR_IN serveraddr, clientaddr;
int len;

void initial()
{
	len = sizeof(sockaddr);
    clientaddr.sin_addr.S_un.S_addr = inet_addr(CLIENT_IP);
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(CLIENT_PORT);
    serveraddr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVER_PORT);
}
int getrand()
{
    return rand()%100;
}
u_short cksum(u_short* mes, int size) {
    int count = (size + 1) / 2;
    u_short* buf = (u_short*)malloc(size + 1);
    memset(buf, 0, size + 1);
    memcpy(buf, mes, size);
    u_long sum = 0;
    while (count--) {
        sum += *buf++;
        if (sum & 0xffff0000) {
            sum &= 0xffff;
            sum++;
        }
    }
    return ~(sum & 0xffff);
}
void create_ClientSocket(SOCKET &Client)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "【ERROR】WSAStartup failed:" << WSAGetLastError << endl;
        return;
    }
    Client = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(Client, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        cout << "【ERROR】bind error:" << WSAGetLastError << endl;
        WSACleanup();
        return;
    }
    else
    {
        cout << "【消息】成功创建客户端！" << endl;
    }
}
void create_ServerSocket(SOCKET &Server)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "【ERROR】WSAStartup failed:" << WSAGetLastError << endl;
        return;
    }
    Server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (bind(Server, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        cout << "【ERROR】bind error:" << WSAGetLastError << endl;
        WSACleanup();
        return;
    }
    else
    {
        cout << "【消息】成功创建服务器！" << endl;
    }
}