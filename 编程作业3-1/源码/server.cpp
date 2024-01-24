#include"../Udp.h"

SOCKET Server;
char filepath[20];
ofstream fout;
int messagenum;
clock_t timestart;
clock_t timeend;
bool recFileName();
bool recFiledata();



void recvconnect()
{
    //第一次握手
    int iMode = 1; //非阻塞
    ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
    cout << "【消息】服务器等待连接！" << endl;
    message recvMsg, sendMsg;
    while (true)
    {
        message msg;
        if (recvfrom(Server, (char*)&msg, BUFFER, 0, (SOCKADDR*)&clientaddr, &len) == -1 || cksum((u_short*)&msg, sizeof(msg)) != 0) {
            recvMsg= message();
        }
        else {
            recvMsg = msg;
        }
        if (recvMsg.SYN==true)
        {
            cout << "【消息】收到第一次握手！" << endl;
            break;
        }
    }
    //第二次握手
    sendMsg.SYN=true; //【SYN，ACK】
    sendMsg.ACK=true;
    sendMsg.ack = recvMsg.seq + 1;   // 确认号等于收到的序列号+1
    sendMsg.seq = getrand();  //随机获取序列号
    sendMsg.checksum = 0;//将校验和置为0
    sendMsg.checksum = cksum((u_short*)&sendMsg, sizeof(sendMsg));//计算校验和
    if (sendto(Server, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "【ERROR】send error:" << WSAGetLastError << endl;
    }
    else {
        cout << "【消息】发送第二次握手！" << endl;
    }
    //第三次握手
    clock_t start=clock();
    clock_t end;
    while (true) {
        message msg;
        if (recvfrom(Server, (char*)&msg, BUFFER, 0, (SOCKADDR*)&clientaddr, &len) == -1 || cksum((u_short*)&msg, sizeof(msg)) != 0) {
            recvMsg= message();
        }
        else{
            recvMsg = msg;
        }
        end = clock();
        if ((end - start) > MAXTIME) {
            cout << "【ERROR】等待时间太长，取消连接！" << endl;
            return recvconnect();
        }
        if (recvMsg.ACK==true && recvMsg.ack == sendMsg.seq + 1) {
            break;
        }
    }
    cout << "【消息】接收到第三次握手，连接成功！" << endl;
}
void closeconnect(message msg) {
    //第二次挥手
    message sendMsg1;
    sendMsg1.ACK=true; //【ACK】
    sendMsg1.ack = msg.seq + 1;//确认号等于序列号+1
    sendMsg1.checksum = 0;//将校验和置为0
    sendMsg1.checksum = cksum((u_short*)&sendMsg1, sizeof(sendMsg1));//计算校验和
    if (sendto(Server, (char*)&sendMsg1, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "【ERROR】send error:" << WSAGetLastError << endl;
    }
    else {
        cout << "【消息】发送第二次挥手！" << endl;
    }
    //第三次挥手
    message sendMsg2;
    sendMsg2.FIN = true;//【FIN】
    sendMsg2.seq = 65534;//序列号
    sendMsg2.checksum = 0;//将校验和置为0
    sendMsg2.checksum = cksum((u_short*)&sendMsg2, sizeof(sendMsg2));//计算校验和
    if (sendto(Server, (char*)&sendMsg2, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
        cout << "【ERROR】send error:" << WSAGetLastError << endl;
    }
    else {
        cout << "【消息】发送第三次挥手！" << endl;
    }
    //第四次挥手
    message recvMsg;
    clock_t start = clock();
    clock_t end;
    while (true) {
        end = clock();
        if ((end - start) > MAXTIME) {
            end = clock();
            if ((start - end) > disconnecttime) {
                cout << "【ERROR】等待时间太长，退出连接" << endl;
                recFileName();
                return;
            }
        }
        message msg;
        if (recvfrom(Server, (char*)&msg, BUFFER, 0, (SOCKADDR*)&clientaddr, &len) == -1 || cksum((u_short*)&msg, sizeof(msg)) != 0) {
             recvMsg= message();
        }
        else {
            recvMsg = msg;
        }
        if (recvMsg.ACK == true && recvMsg.ack == sendMsg2.seq + 1) {
            cout << "【消息】断开连接成功！" << endl;
            break;
        }

    }

}
bool recFileName() {
    cout << "服务器正在等待中......" << endl;
    message msg, sendMsg;
    clock_t start = clock();
    clock_t end;
    int iMode = 0; //阻塞
    ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//阻塞设置
    while (true) {
        end = clock();
        if ((end - start) > MAXTIME) {
            cout << "【ERROR】连接超时，重新等待接收文件......" << endl;
            start = clock();
            continue;
        }
        message tempmsg;
        if (recvfrom(Server, (char*)&tempmsg, BUFFER, 0, (SOCKADDR*)&clientaddr, &len) == -1 || cksum((u_short*)&tempmsg, sizeof(tempmsg)) != 0) {
           msg= message();
        }
        else {
            msg = tempmsg;
        }
        if (msg.FIN) {
            cout << "【消息】收到第一次挥手！" << endl;
            closeconnect(msg); //断开连接
            break;
        }
        if (msg.START) {
            memset(filepath,0, filepathlen);
            memcpy(filepath, msg.data, msg.len);
            fout.open(filepath, std::ios::out | std::ios::binary);
            if (!fout.is_open())
            {
                cout << "【ERROR】文件打开失败！！！" << endl;
                exit(1);
            }else{
                    cout << "【消息】文件名为：" << filepath << endl;
            }
            messagenum = msg.num;
            sendMsg.ACK=true;
            sendMsg.ack = msg.seq;
            sendMsg.checksum = 0;//将校验和置为0
            sendMsg.checksum = cksum((u_short*)&sendMsg, sizeof(sendMsg));//计算校验和
            if (sendto(Server, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
                cout << "【ERROR】发送错误了!!！" << endl;
                return false;
            }
            else {
                cout << "【消息】文件共有" << msg.num << "个数据包。" << endl;
                timestart = clock();
                recFiledata();
                return true;
            }
        }
    }
}
bool recFiledata() {
    cout << "开始接收文件内容......." << endl;
    message recvMsg, sendMsg;
    int seq = 1;
    int iMode = 1; //非阻塞
    ioctlsocket(Server, FIONBIO, (u_long FAR*) & iMode);//非阻塞设置
    for (int i = 0; i < messagenum; i++) {
        clock_t start = clock();
        clock_t end;
        while (1) {
            end = clock();
            if (end - start > MAXTIME) {
                cout << "【ERROR】传输超时，接收失败，进行重新接收！" << endl;
                start = clock();
                continue;
            }
            message msg;
            if (recvfrom(Server, (char*)&msg, BUFFER, 0, (SOCKADDR*)&clientaddr, &len) == -1 || cksum((u_short*)&msg, sizeof(msg)) != 0) {
                recvMsg = message();
            }
            else {
                recvMsg = msg;
            }

            if (recvMsg.seq == seq) {
                cout << "收到seq为" << recvMsg.seq << "的数据包" << endl;
                cout << "checksum=" << recvMsg.checksum << ", len=" << recvMsg.len << endl;
                sendMsg.ACK = true;
                sendMsg.ack = recvMsg.seq;
                sendMsg.checksum = 0;//将校验和置为0
                sendMsg.checksum = cksum((u_short*)&sendMsg, sizeof(sendMsg));//计算校验和
                if (sendto(Server, (char*)&sendMsg, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
                    cout << "【ERROR】send error:" << WSAGetLastError << endl;
                }
                else {
                    cout << "向服务端发送确认数据包!" << endl;
                }
                cout << endl;
                fout.write(recvMsg.data, recvMsg.len);
                break;
            }
        }
        if (recvMsg.END) {
            cout << "【消息】接收文件成功！！！" << endl << endl;
            fout.close();
            fout.clear();
            timeend = clock();
            double sendtime = (double)(timeend - timestart) / CLOCKS_PER_SEC;
            cout << "传输总时间" << sendtime << "s" << endl;
            cout << "吞吐率" << (double)(messagenum) * sizeof(message) * 8 / sendtime / datasize << "kbps" << endl;
            return recFileName();
        }
        seq++;
    }
}

int main()
{
    initial();
    create_ServerSocket(Server);
    recvconnect();
    recFileName();
    //关闭套接字
    closesocket(Server);
    WSACleanup();
    system("pause");
    return 0;
}