#include"TcpSocket.h"

int main()
{
	//�������
	open_Socket();
	//��������ʼ�������socket
	SOCKET serverfd = create_ServerSocket();
	cout << "[��Ϣ]������������ɹ���" << endl;
	int i = 0;
	pthread_create(&send_thread, NULL, serv_send, NULL);
	while (true)
	{
		sockaddr_in addr;
		int addrlen = sizeof(addr);
		//���пͻ�����������ʱ���ܿͻ���
		SOCKET clientfd = accept(serverfd,(sockaddr*)&addr, &addrlen);
		if (clientfd == INVALID_SOCKET)
		{
			cout << "accept failed" << WSAGetLastError << endl;
		}
		All_Clientfd.push_back(clientfd);
		cout << "[��Ϣ]���ͻ������ӵ�����ˣ�" << "IP��ַ��" << inet_ntoa(addr.sin_addr) << " �˿ںţ�" << ntohs(addr.sin_port) << endl;
		//���տͻ�������
		char name[MAX_NAME_LEN] = { 0 };
		if (recv(clientfd, name, MAX_NAME_LEN, NULL) == SOCKET_ERROR)
		{
			cout << "recv name failed:" << WSAGetLastError << endl;
		}
		cout << "�û��ǳƣ�" << name << endl;
		serv_thread serv_thread;
		serv_thread.sockaddr = addr;
		serv_thread.clientfd = clientfd;
		serv_thread.name = name;
		//�������̱߳߷�����Ϣ�߽�����Ϣ
		pthread_t recv_thread;
		pthread_create(&recv_thread, NULL, serv_recv, (void*)&serv_thread);
	}
	// �رշ����
	closesocket(serverfd);
	//�ر������
	close_Socket();
	cout << "-------end---------" << endl;
	return 0;
}