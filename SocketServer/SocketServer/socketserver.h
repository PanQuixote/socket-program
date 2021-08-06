#ifndef SERVER_H
#define SERVER_H

#include <Winsock2.h>
#include <windows.h>

#include <string>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

#define IP_BUF_SIZE 129

// how to use it:
// Server s;
// s.WaitForClient();
// do something in handleMessage();

class SocketServer
{
public:
    SocketServer();
    ~SocketServer();
    SocketServer(const SocketServer &) = delete;
    SocketServer & operator=(const SocketServer &) = delete;
    void WaitForClient();

	bool sendMessage(string msg, SOCKET receiver_socket = -1);

	bool broadcastMessage(string msg);

	int handleMessage(string msg, SOCKET sender_socket);
private:
    WORD winsock_ver;
    WSADATA wsa_data;
    SOCKET sock_svr;
    SOCKET sock_clt;
    HANDLE h_thread;
    SOCKADDR_IN addr_svr;
    SOCKADDR_IN addr_clt;
    int ret_val;
    int addr_len;
    char buf_ip[IP_BUF_SIZE];
};

// used to be thread's arg
class ThreadArg {
public:
	ThreadArg(SocketServer* pObj, int socket)
		:m_pObj(pObj), m_socket(socket)
	{

	}
public:
	SocketServer* m_pObj;
	int m_socket;
};
#endif
