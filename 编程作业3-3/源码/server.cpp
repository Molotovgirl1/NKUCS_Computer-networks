#include"../Udp.h"

SOCKET Server;
char filepath[20];
ofstream fout;
int messagenum;
clock_t timestart;
clock_t timeend;
bool recFileName();
bool recFiledata();

int lastlen;
int recvbase;
int recvtop;
char buffer[statussize][datasize];
int status[statussize] = { 0 };
mutex coutMutex; //输出锁
int savefile();
int recvFileData();

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
            msg = message();
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
            memset(filepath, 0, filepathlen);
            memcpy(filepath, msg.data, msg.len);
            fout.open(filepath, std::ios::out | std::ios::binary);
            if (!fout.is_open())
            {
                cout << "【ERROR】文件打开失败！！！" << endl;
                exit(1);
            }
            else {
                cout << "【消息】文件名为：" << filepath << endl;
            }
            messagenum = msg.num;
            sendMsg.ACK = true;
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
                recvFileData();
                return true;
            }
        }
    }
}
int recvThread() {
    message recv;
    while (true) {
        if (recvfrom(Server, (char*)&recv, BUFFER, 0, (SOCKADDR*)&clientaddr, &len) == -1 || cksum((u_short*)&recv, sizeof(recv)) != 0) {
            recv = message();
        }
        else {
            int ack = recv.seq;
            message send;
            send.ACK = true;
            send.ack = ack;
            send.checksum = 0;
            send.checksum = cksum((u_short*)&send, sizeof(send));
            //发送结束
            if (recv.END) {
                unique_lock<mutex> lock(coutMutex);
                cout << "【结束】发送结束数据包" << endl;
                lock.unlock();
                if (sendto(Server, (char*)&send, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
                    cout << "发送错误了!!" << endl;
                }
                return 0;
            }
            //接收到数据包，但确认包可能丢失
            if (status[ack] == 1 || ack < recvbase) {
                unique_lock<mutex> lock(coutMutex);
                cout << "【接收】收到无效包裹" << endl;
                lock.unlock();
                if (sendto(Server, (char*)&send, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
                    cout << "发送错误了!!" << endl;
                }
            }
            //接收到数据包
            if (status[ack] == 0&&ack<=recvtop) {
                unique_lock<mutex> lock(coutMutex);
                cout << "【发送】发送了确认号为" << ack << "的数据包" << endl;
                lock.unlock();
                if (sendto(Server, (char*)&send, BUFFER, 0, (SOCKADDR*)&clientaddr, sizeof(SOCKADDR)) == (SOCKET_ERROR)) {
                    cout << "发送错误了!!" << endl;
                }
                if (ack == messagenum - 1) {
                    lastlen = recv.len;
                }
                memcpy(buffer[ack], recv.data, recv.len);
                status[ack] = 1;
            }
        }
    }
}
int slideThread() {
    while (true) {
        if(status[recvbase] == 1) {
            if (recvbase == messagenum - 1) { //收到最后一个数据包
                return 0;
            }
            recvbase++;
            if (recvtop < messagenum - 1) {
                recvtop++;
            }
            unique_lock<mutex> lock(coutMutex);
            cout << "【滑动】滑动窗口移动,recvbase:" << recvbase << ",recvtop:" << recvtop << endl;
            lock.unlock();
        }
    }
}
int recvFileData() {
    recvbase = 0;
    recvtop = (messagenum - 1 <= WindowSize-1) ? messagenum - 1 : WindowSize - 1;
    cout << "开始接收文件内容！" << endl;
    //创建一个接收线程
    thread recv_thread(recvThread);
    //创建一个滑动线程
    thread slide_thread(slideThread);
    recv_thread.join();
    slide_thread.join();
    cout << "接收完成" << endl;
    return savefile();
}

int savefile() {
    int i;
    for (i = 0; i < messagenum - 1; i++) {
        fout.write(buffer[i], datasize);
    }
    fout.write(buffer[i], lastlen);
    fout.close();
    fout.clear();
    cout << "写文件完成" << endl;
    return recFileName();
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