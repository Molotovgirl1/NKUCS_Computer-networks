#include"TcpSocket.h"

int main()
{
	//打开网络库
	open_Socket();
	//创建并初始化服务端socket
	SOCKET serverfd = create_ServerSocket();
	cout << "[信息]：服务端启动成功！" << endl;
	int i = 0;
	pthread_create(&send_thread, NULL, serv_send, NULL);
	while (true)
	{
		sockaddr_in addr;
		int addrlen = sizeof(addr);
		//当有客户端请求连接时接受客户端
		SOCKET clientfd = accept(serverfd,(sockaddr*)&addr, &addrlen);
		if (clientfd == INVALID_SOCKET)
		{
			cout << "accept failed" << WSAGetLastError << endl;
		}
		All_Clientfd.push_back(clientfd);
		cout << "[信息]：客户端连接到服务端！" << "IP地址：" << inet_ntoa(addr.sin_addr) << " 端口号：" << ntohs(addr.sin_port) << endl;
		//接收客户端名字
		char name[MAX_NAME_LEN] = { 0 };
		if (recv(clientfd, name, MAX_NAME_LEN, NULL) == SOCKET_ERROR)
		{
			cout << "recv name failed:" << WSAGetLastError << endl;
		}
		cout << "用户昵称：" << name << endl;
		serv_thread serv_thread;
		serv_thread.sockaddr = addr;
		serv_thread.clientfd = clientfd;
		serv_thread.name = name;
		//创建多线程边发送消息边接收消息
		pthread_t recv_thread;
		pthread_create(&recv_thread, NULL, serv_recv, (void*)&serv_thread);
	}
	// 关闭服务端
	closesocket(serverfd);
	//关闭网络库
	close_Socket();
	cout << "-------end---------" << endl;
	return 0;
}