#include"../socket_com/socket_com/TcpSocket.h"

int main()
{
	//�������
	open_Socket();
	//����socket�����������������
	SOCKET clientfd = create_ClientSocket();
	//�����˷�������
	cout << "����������ǳƣ�";
	char name[MAX_NAME_LEN];
	cin.getline(name,MAX_NAME_LEN);
	if (send(clientfd, name, MAX_NAME_LEN, NULL) == SOCKET_ERROR)
	{
		cout << "send name failed:" << WSAGetLastError << endl;
	}
	client_thread client;
	client.clientfd= clientfd;
	client.name = name;
	//�������̱߳߷�����Ϣ�߽�����Ϣ
	pthread_t send_thread;
	pthread_t recv_thread;
	pthread_create(&recv_thread, NULL, client_recv, (void*)&client);
	pthread_create(&send_thread, NULL, client_send, (void*)&client);
	//���̵߳ȴ��������߳�
	pthread_join(send_thread, NULL);
	//�رտͻ���
	closesocket(clientfd);
	//�ر������
	close_Socket();
	cout << "-------end---------" << endl;
	return 0;
}