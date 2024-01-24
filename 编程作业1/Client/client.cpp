#include"../socket_com/socket_com/TcpSocket.h"

int main()
{
	//打开网络库
	open_Socket();
	//创建socket并与服务器建立连接
	SOCKET clientfd = create_ClientSocket();
	//向服务端发送名字
	cout << "请输入你的昵称：";
	char name[MAX_NAME_LEN];
	cin.getline(name,MAX_NAME_LEN);
	if (send(clientfd, name, MAX_NAME_LEN, NULL) == SOCKET_ERROR)
	{
		cout << "send name failed:" << WSAGetLastError << endl;
	}
	client_thread client;
	client.clientfd= clientfd;
	client.name = name;
	//创建多线程边发送消息边接收消息
	pthread_t send_thread;
	pthread_t recv_thread;
	pthread_create(&recv_thread, NULL, client_recv, (void*)&client);
	pthread_create(&send_thread, NULL, client_send, (void*)&client);
	//主线程等待回收子线程
	pthread_join(send_thread, NULL);
	//关闭客户端
	closesocket(clientfd);
	//关闭网络库
	close_Socket();
	cout << "-------end---------" << endl;
	return 0;
}