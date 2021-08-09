#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include<iostream>
#include<winsock.h>
#include<string>

#pragma comment(lib,"ws2_32.lib")

#define MAX_LENGTH 1000

using namespace std;


class SocketClient
{
public:
  explicit SocketClient();

public:
  bool init();

  bool close();

  bool sendMessage(string msg);

  int handleMessage(string msg);

private:

  //定义发送缓冲区和接受缓冲区
  char send_buf[MAX_LENGTH];
  char recv_buf[MAX_LENGTH];

  //定义服务端套接字，接受请求套接字
  SOCKET server_socket;

  //服务端地址客户端地址
  SOCKADDR_IN server_addr;

  bool initFlag;

  HANDLE h_thread;

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

