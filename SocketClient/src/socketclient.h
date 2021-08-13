#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include<iostream>
#include<winsock.h>
#include<string>

#pragma comment(lib,"ws2_32.lib")

#define MSG_BUF_SIZE 1024

#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 8888

using namespace std;

// how to use:
// client c;
// c.start()

class SocketClient
{

public:
	explicit SocketClient(bool start_after_init = false,
		bool block_to_wait_enter_instruct = false);

public:

	bool init();

	bool start(bool block_to_wait_enter_instruct = false);

	bool close();

	bool setIpAndPort(string ip = "127.0.0.1", int port = 8888);

	bool sendMessage(string msg);

	int handleMessage(string msg);


public:

	//定义发送缓冲区和接受缓冲区
	char send_buf[MSG_BUF_SIZE];
	char recv_buf[MSG_BUF_SIZE];

	//定义服务端套接字，接受请求套接字
	SOCKET server_socket;

	//服务端地址客户端地址
	SOCKADDR_IN server_addr;

	bool initFlag;

	HANDLE h_thread;

	string m_ip;
	int m_port;

};

// used to be thread's arg
class ThreadArg {
public:
	ThreadArg(SocketClient* pObj, int socket)
		:m_pObj(pObj), m_socket(socket)
	{

	}
public:
	SocketClient* m_pObj;
	int m_socket;
};

#endif // SOCKETCLIENT_H

