#include"../Udp.h"

SOCKET Client;
int messagenum;
char filepath[20];
clock_t timestart;
clock_t timeend;
int filelen;
ifstream fin;
bool SEND_ACK(message send, int seq);
void sendFiledata();


void sendconnect()
{
    //第一次握手
    int iMode = 1; 
    ioctlsocket(Client, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
    cout << "【消息】开始连接！发送第一次握手！" << endl;
    message recvMsg, sendMsg;
    sendMsg.SYN=true; //【SYN】
    sendMsg.seq = getrand(); //随机产生一个序列号
    sendMsg.checksum=0;//将校验和置为0
    sendMsg.checksum= cksum((u_short*)&sendMsg, sizeof(sendMsg));//计算校验和
    if (sendto(Client, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "【ERROR】send error:" <<WSAGetLastError<< endl;
    }
    //第二次握手
    clock_t start = clock();
    clock_t end;
    while (true)
    {
        message msg;
        if (recvfrom(Client, (char*)&msg, BUFFER, 0, (SOCKADDR*)&serveraddr, &len) == -1 || cksum((u_short*)&msg, sizeof(msg)) != 0) {
            recvMsg = message();
        }
        else {
            recvMsg=msg;
        }
        //超时重传
        end = clock();
        if ((end - start)> MAXTIME) {
            cout << "【ERROR】连接超时,请保持网络通畅和服务端正常运行！" << endl;
            if (sendto(Client, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
                cout << "【ERROR】send error:" << WSAGetLastError << endl;
            }
            start = clock();
            continue;
        }
        //接收确认
        if (recvMsg.ACK&& recvMsg.SYN && recvMsg.ack == sendMsg.seq + 1) {
            cout << "【消息】接收到第二次握手!" << endl;
            break;
        }

    }
    //第三次握手
    sendMsg.ACK = true;
    sendMsg.seq = recvMsg.ack; //序列号等于收到的确认号
    sendMsg.ack = recvMsg.seq + 1; //确认号等于收到的序列号+1
    cout << "【消息】发送第三次握手！" << endl;
    sendMsg.checksum = 0;//将校验和置为0
    sendMsg.checksum = cksum((u_short*)&sendMsg, sizeof(sendMsg));//计算校验和
    if (sendto(Client, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "【ERROR】send error:" << WSAGetLastError << endl;
    }
}
void closeconnect() {  // 断开连接
    //第一次挥手
    message recvMsg, sendMsg1;
    sendMsg1.FIN = true; //【FIN】
    sendMsg1.seq = 65533;//序列号
    sendMsg1.checksum = 0;//将校验和置为0
    sendMsg1.checksum = cksum((u_short*)&sendMsg1, sizeof(sendMsg1));//计算校验和
    if (sendto(Client, (char*)&sendMsg1, BUFFER, 0, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "【ERROR】send error:" << WSAGetLastError << endl;
    }
    else {
        cout << "【消息】断开连接！发送第一次挥手！" << endl;
    }
    //第二次挥手
    clock_t start = clock();
    clock_t end;
    while (true) {
        end = clock();
        if ((start - end) > disconnecttime) {
            cout << "【ERROR】等待时间太长，退出连接" << endl;
            return closeconnect();
        }
        message msg;
        if (recvfrom(Client, (char*)&msg, BUFFER, 0, (SOCKADDR*)&serveraddr, &len) == -1 || cksum((u_short*)&msg, sizeof(msg)) != 0) {
            recvMsg = message();
        }
        else {
            recvMsg = msg;
        }
        if (recvMsg.ACK == true && recvMsg.ack == sendMsg1.seq + 1) {
            cout << "【消息】客户端收到第二次挥手" << endl;
            break;
        }
    }
    //第三次挥手
    start = clock();
    while (true) {
        end = clock();
        if ((start - end) > disconnecttime) {
            cout << "【ERROR】等待时间太长，退出连接" << endl;
            return closeconnect();
        }
        message msg;
        if (recvfrom(Client, (char*)&msg, BUFFER, 0, (SOCKADDR*)&serveraddr, &len) == -1 || cksum((u_short*)&msg, sizeof(msg)) != 0) {
            recvMsg = message();
        }
        else {
            recvMsg = msg;
        }
        if (recvMsg.FIN==true) {
            cout << "【消息】客户端收到第三次挥手" << endl;
            break;
        }
    }
    //第四次挥手
    message sendMsg2;
    sendMsg2.ACK = true; //【ACK】
    sendMsg2.ack = recvMsg.seq + 1;//确认号等于序列号+1
    sendMsg2.checksum = 0;//将校验和置为0
    sendMsg2.checksum = cksum((u_short*)&sendMsg2, sizeof(sendMsg2));//计算校验和
    if (sendto(Client, (char*)&sendMsg2, BUFFER, 0, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "【ERROR】send error:" << WSAGetLastError << endl;
    }
    else {

        cout << "【消息】断开连接成功！" << endl;
    }
}
void sendFileName() {
    cout << "请输入要发送的文件名：";
    message name_msg;
    memset(filepath, 0, filepathlen);
    string filename;
    cin >> filename;
    int count = 1;
    if (filename == "1.jpg" || filename == "2.jpg" || filename == "3.jpg" || filename == "helloworld.txt") {
        strcpy(filepath, filename.c_str());
        fin.open(filepath, ifstream::in | ios::binary);// 以二进制方式打开文件
        fin.seekg(0, std::ios_base::end);  // 将文件流指针定位到流的末尾
        filelen = fin.tellg();
        messagenum = filelen / datasize + (filelen % datasize != 0);
        cout << "文件大小为" << filelen << "Bytes,总共有" << messagenum << "个数据包" << endl;
        fin.seekg(0, std::ios_base::beg);  // 将文件流指针定位到流的开始
        memcpy(name_msg.data, filepath, strlen(filepath));
        name_msg.len = strlen(filepath);
        name_msg.num = messagenum;
        name_msg.START = true;
        if (SEND_ACK(name_msg, 0) == false) {
            cout << "【ERROR】发出文件名失败！" << endl;
           //重新发送
            while (count <= 10) {
                if (SEND_ACK(name_msg, 0) == true) {
                    cout << "第" << count << "次重新发送seq为" << 0 << "的数据包成功！！！" << endl;
                    break;
                }
                else {
                    cout << "第" << count << "次重新发送seq为" << 0 << "的数据包失败！！！" << endl;
                }
            }
            if (count == 10) {
                closeconnect();
                return;
            }
        }
        else {
            cout << "【消息】发送文件名成功！" << endl;
            timestart = clock();
            sendFiledata(); //发送文件
        }

    }
    else {
        cout << "【提示】文件不存在，断开连接！" << endl;
        closeconnect();
        return;
    }
}
void sendFiledata() {
    cout << "开始发送文件内容！" << endl;
    message msg;
    int seq = 1;
    int count = 0;
    while (filelen) {
        if (filelen > datasize)
        {
            fin.read(msg.data, datasize);
            msg.len = datasize;
            filelen -= datasize;
        }
        else
        {
            fin.read(msg.data, filelen);
            msg.len = filelen;
            msg.END = true;
            filelen = 0;
        }
        if (SEND_ACK(msg, seq) == false) {
            cout << "发送seq为" << seq << "的数据包失败！！！" << endl;
            //重新发送
            while (count <= 10) {
                if (SEND_ACK(msg, seq)== true){
                    cout<<"第"<<count << "次重新发送seq为" << seq << "的数据包成功！！！" << endl;
                    break;
                }
                else {
                    cout << "第"<<count<<"次重新发送seq为" << seq << "的数据包失败！！！" << endl;
                }
            }
            if (count == 10) {
                closeconnect();
                return;
            }
        }
        seq++;
    }
    cout << "【消息】成功发送文件！" << endl;
    timeend = clock();
    double sendtime = (double)(timeend - timestart) / CLOCKS_PER_SEC;
    cout << "传输总时间" << sendtime << "s" << endl;
    cout << "吞吐率" << (double)(messagenum) * sizeof(message) * 8 / sendtime / datasize << "kbps" << endl;
    fin.close();
    fin.clear();
    sendFileName();  // 准备发送下一个文件
}
bool SEND_ACK(message sendMsg, int seq)
{
    message recvMsg;
    sendMsg.seq = seq;
    sendMsg.checksum = 0;//将校验和置为0
    sendMsg.checksum = cksum((u_short*)&sendMsg, sizeof(sendMsg));//计算校验和
    if (sendto(Client, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "【ERROR】send error:" << WSAGetLastError << endl;
    }
    cout << "发送seq为" << seq << "的数据包成功！！！" << endl;
    cout << "checksum=" << sendMsg.checksum << ", len=" << sendMsg.len << endl;
    int iMode = 1; //非阻塞
    ioctlsocket(Client, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
    int count = 0;
    clock_t start = clock();
    clock_t end;
    while (true) {
        message msg;
        if (recvfrom(Client, (char*)&msg, BUFFER, 0, (SOCKADDR*)&serveraddr, &len) == -1 || cksum((u_short*)&msg, sizeof(msg)) != 0) {
            recvMsg = message();
        }
        else {
            recvMsg = msg;
        }
        end = clock();
        if ((end - start )> MAXTIME) { //超时重传
            cout << "应答超时，重新发送数据包" << endl;
            if (sendto(Client, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&serveraddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
                cout << "【ERROR】send failed:" << WSAGetLastError << endl;
            }
            count++;
            cout << "尝试重新发送第" << count << "次数据包" << endl;
            if (count >= 10) {
                break;
            }
            start = clock();
        }
        if (recvMsg.ACK==true && recvMsg.ack == seq) {
            cout << "收到服务器的确认数据包！" << endl;
            cout << "checksum=" << recvMsg.checksum << ", len=" << recvMsg.len << endl;
            cout << endl;
            return true;
        }
    }
    return false;
}

int main()
{
    initial();
    create_ClientSocket(Client);
    sendconnect();
    sendFileName();
    closesocket(Client);
    WSACleanup();
    system("pause");
    return 0;

}