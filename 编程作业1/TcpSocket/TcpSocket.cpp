#include"TcpSocket.h"

vector<SOCKET>All_Clientfd;
pthread_t send_thread;
bool open_Socket()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2,2),&wsadata)!= 0)
	{
		cout << "WSAStartup failed:" << WSAGetLastError << endl;
		return false;
	}
	return true;
}
bool close_Socket()
{
	if (WSACleanup() != 0)
	{
		cout << "WSACleanup failed:" << WSAGetLastError << endl;
		return false;
	}
	return true;
}

SOCKET create_ServerSocket()
{
	//创建socket
	SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == INVALID_SOCKET)
	{
		cout << "socket failed:" << WSAGetLastError << endl;
		return INVALID_SOCKET;
	}
	//给socket绑定ip地址和端口号
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERPORT);
	addr.sin_addr.S_un.S_addr = inet_addr(IP);
	if (bind(fd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		cout << "bind failed:" << WSAGetLastError << endl;
		return false;
	}
	//监听客户端连接
	listen(fd, 10);
	return fd;
}
SOCKET create_ClientSocket()
{
	//创建客户端的socket
	SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == INVALID_SOCKET)
	{
		cout << "client socket failed:" << WSAGetLastError << endl;
		return INVALID_SOCKET;
	}
	//与服务器建立连接
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERPORT);
	addr.sin_addr.S_un.S_addr = inet_addr(IP);
	if(connect(fd, (sockaddr*)&addr, sizeof(addr) )== SOCKET_ERROR)
	{
		cout << "connect failed:" << WSAGetLastError << endl;
		return false;
	}
	cout << "[消息]：连接服务端成功！" << endl;
	return fd;
}

void* client_send(void* arg)
{
	client_thread* client = (client_thread*)arg;
	char endbuf[MAX_NAME_LEN] = { 0 };
	strcpy(endbuf, ENDSIGNAL);
	while (1)
	{
		//发送消息给服务端
		char sendbuf[MAX_SEND_LEN] = { 0 };
		cin.getline(sendbuf, MAX_SEND_LEN);
		if (send(client->clientfd, sendbuf, MAX_SEND_LEN, NULL) == SOCKET_ERROR)
		{
			cout << "client send failed:" << WSAGetLastError << endl;
		}
		if (strcmp(sendbuf, endbuf) == 0)
		{
			return NULL;
		}
	}
	return NULL;
}
void* client_recv(void* arg)
{
	client_thread* client = (client_thread*)arg;
	while (1)
	{
		//接收服务端的消息
		char recvbuf[MAX_REC_LEN] = { 0 };
		if (recv(client->clientfd, recvbuf, MAX_REC_LEN, NULL) > 0)
		{
			cout  << recvbuf << endl;
		}
	}
	return NULL;
}

void* serv_send(void* arg)
{
	while (1)
	{
		//向所有客户端发送消息
		char content[MAX_CONTENT_LEN] = { 0 };
		strcpy(content, "server:");
		char sendbuf[MAX_SEND_LEN];
		cin.getline(sendbuf, MAX_SEND_LEN);
		strcat(content, sendbuf);
		if (All_Clientfd.size() == 0)
		{
			cout << "[提示]：没有连接的客户端" << endl;
		}
		else {
			for (int i = 0;i < All_Clientfd.size();i++)
			{
				if (send(All_Clientfd[i], content, MAX_CONTENT_LEN, 0) == SOCKET_ERROR)
				{
					cout << "service send failed:" << WSAGetLastError << endl;
					cout << "[信息]：可能存在客户已退出聊天室！" << endl;
					closesocket(All_Clientfd[i]);
					All_Clientfd.erase(All_Clientfd.begin()+i);
					return NULL;
				}
			}
		}
	}
	return NULL;
}
void* serv_recv(void* arg)
{
	serv_thread* server = (serv_thread*)arg;
	const SOCKET clientfd = server->clientfd;
	char name[MAX_NAME_LEN] = {0};
	strcpy(name, server->name);
	char endbuf[MAX_REC_LEN] = { 0 }; 
	strcpy(endbuf,ENDSIGNAL);
	while (1)
	{
		char recbuf[MAX_REC_LEN] = { 0 };
		//接收客户端消息
		if (recv(clientfd, recbuf, MAX_REC_LEN, NULL) > 0)
		{
			if (strcmp(recbuf, endbuf) == 0)
			{
				cout << "[提示]："<<name<<"已经退出聊天！" << endl;
				closesocket(clientfd);
				All_Clientfd.erase(remove(All_Clientfd.begin(), All_Clientfd.end(), clientfd), All_Clientfd.end());
				return NULL;
			}
			cout << name << "：" << recbuf << endl;
			//向所有客户端广播消息
			char content[MAX_CONTENT_LEN] = { 0 };
			strcpy(content, name);
			strcat(content, ":");
			strcat(content, recbuf);
			for (int i = 0;i < All_Clientfd.size();i++)
			{
				if (send(All_Clientfd[i], content, MAX_CONTENT_LEN, NULL) == SOCKET_ERROR)
				{
					cout << "broadcast failed:" << WSAGetLastError << endl;
				}
			}
		}
	}
	return NULL;
}
