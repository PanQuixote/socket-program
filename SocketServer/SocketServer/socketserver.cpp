#include "socketserver.h"
#include <iostream>
#include <WS2tcpip.h>

using std::cerr;
using std::cout;
using std::endl;


SocketServer::SocketServer()
{
    cout << "Initializing server...\n";
    winsock_ver = MAKEWORD(2, 2);
    addr_len = sizeof(SOCKADDR_IN);
    addr_svr.sin_family = AF_INET;
    addr_svr.sin_port = ::htons(SERVER_PORT);
    addr_svr.sin_addr.S_un.S_addr = ADDR_ANY;
    memset(buf_ip, 0, IP_BUF_SIZE);


    ret_val = ::WSAStartup(winsock_ver, &wsa_data);
    if (ret_val != 0)
    {
        cerr << "WSA failed to start up!Error code: " << ::WSAGetLastError() << "\n";
        system("pause");
        exit(1);
    }
    cout << "WSA started up successfully...\n";


    sock_svr = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_svr == INVALID_SOCKET)
    {
        cerr << "Failed to create server socket!Error code: " << ::WSAGetLastError() << "\n";
        ::WSACleanup();
        system("pause");
        exit(1);
    }
    cout << "Server socket created successfully...\n";


    ret_val = ::bind(sock_svr, (SOCKADDR*)&addr_svr, addr_len);
    if (ret_val != 0)
    {
        cerr << "Failed to bind server socket!Error code: " << ::WSAGetLastError() << "\n";
        ::WSACleanup();
        system("pause");
        exit(1);
    }
    cout << "Server socket bound successfully...\n";


	ret_val = ::listen(sock_svr, MAX_CLIENT_COUNT);//SOMAXCONN);
    if (ret_val == SOCKET_ERROR)
    {
        cerr << "Server socket failed to listen!Error code: " << ::WSAGetLastError() << "\n";
        ::WSACleanup();
        system("pause");
        exit(1);
    }
    cout << "Server socket started to listen...\n";


    cout << "Server started successfully..." << endl;
}

SocketServer::~SocketServer()
{
    ::closesocket(sock_svr);
    ::closesocket(sock_clt);
    ::WSACleanup();
    cout << "Socket closed..." << endl;
}

DWORD WINAPI CreateClientThread(LPVOID lpParameter);
//
void SocketServer::WaitForClient()
{
    while (true)
    {
        sock_clt = ::accept(sock_svr, (SOCKADDR*)&addr_clt, &addr_len);
        if (sock_clt == INVALID_SOCKET)
        {
            cerr << "Failed to accept client!Error code: " << ::WSAGetLastError() << "\n";
            ::WSACleanup();
            system("pause");
            exit(1);
        }

        PWSTR buf_ip_p = (PWSTR)buf_ip;
        ::InetNtop(addr_clt.sin_family, &addr_clt, buf_ip_p, IP_BUF_SIZE);
        cout << "A new client connected...IP address: " << buf_ip << ", port number: " << ::ntohs(addr_clt.sin_port) << endl;

		ThreadArg* pArg = new ThreadArg(this, sock_clt);
        h_thread = ::CreateThread(nullptr, 0, CreateClientThread, (LPVOID)pArg, 0, nullptr);
        if (h_thread == NULL)
        {
            cerr << "Failed to create a new thread!Error code: " << ::WSAGetLastError() << "\n";
            ::WSACleanup();
            system("pause");
            exit(1);
        }

        client_manager.addClient(sock_clt, buf_ip);

        ::CloseHandle(h_thread);
    }
}

bool SocketServer::sendMessage(string msg, SOCKET receiver_socket)
{
	if (receiver_socket == -1) {
		return broadcastMessage(msg);
	}

	const char* c = msg.c_str();
	if (send(receiver_socket, c, msg.length(), 0) == SOCKET_ERROR) {
		cout << "fail to send message to client:  socket = " << receiver_socket 
			<< ", index = " << socketIndex(receiver_socket) << endl;
		return false;
	}
	return true;
}

bool SocketServer::broadcastMessage(string msg)
{
	bool flag = true;

	const char* c = msg.c_str();
	for (int i = 0; i < MAX_CLIENT_COUNT; i++) {
        if (client_manager.isValid(i) == false) {
            continue;
        }

		if (send(client_manager.getSocket(i), c, msg.length(), 0) == SOCKET_ERROR) {
			cout << "fail to send message to client:  socket = " << client_manager.getSocket(i)
				<< ", index = " << i << endl;

			flag = false;
		}
	}
	return flag;
}

int SocketServer::handleMessage(string msg, SOCKET sender_socket)
{
	cout << "Message received: " << msg << ", socket = " << sender_socket
		<< ", index = " << socketIndex(sender_socket) << endl;

	if (sendMessage(msg, sender_socket)) {
		return 0;
	}
	else {
		return 1;
	}

	return 0;
}

bool checksock(int s)
{
	fd_set   fds;
	char buf[2];
	int nbread;
	FD_ZERO(&fds);
	FD_SET(s, &fds);
	if (select(s + 1, &fds, (fd_set *)0, (fd_set *)0, NULL) == -1) {
		//log(LOG_ERR,"select(): %s\n",strerror(errno)) ;
		return false;
	}
	if (!FD_ISSET(s, &fds)) {
		//log(LOG_ERR,"select() returns OK but FD_ISSET not\n") ;
		return false;
	}
	/* read one byte from socket */
	nbread = recv(s, buf, 1, MSG_PEEK);
	if (nbread <= 0)
		return false;
	return true;
}

DWORD WINAPI CreateClientThread(LPVOID lpParameter)
{
	ThreadArg* pArg = (ThreadArg*)lpParameter;
	SOCKET sock_clt = pArg->m_socket;
	int socket_index = pArg->m_pObj->socketIndex(sock_clt);
    char buf_msg[MSG_BUF_SIZE];
    int ret_val = 0;
    int snd_result = 0;
    do
    {
		if (checksock(sock_clt) == false) {
            pArg->m_pObj->deleteClient(sock_clt);
			cout << "user " << socket_index
			<< " connection closed..." << endl;
			break;
		}

        memset(buf_msg, 0, MSG_BUF_SIZE);
        ret_val = ::recv(sock_clt, buf_msg, MSG_BUF_SIZE, 0);
        if (ret_val > 0)
        {
            if (strcmp(buf_msg, "exit") == 0)
            {
                cout << "Client " << socket_index
					<< "requests to close the connection..." << endl;
                pArg->m_pObj->deleteClient(sock_clt);
                break;
            }

			string msg = buf_msg;

			pArg->m_pObj->handleMessage(msg, sock_clt);

            //snd_result = ::send(sock_clt, buf_msg, MSG_BUF_SIZE, 0);
            //if (snd_result == SOCKET_ERROR)
            //{
            //    cerr << "Failed to send message to client!Error code: " << ::GetLastError() << "\n";
            //    ::closesocket(sock_clt);
            //    system("pause");
            //    return 1;
            //}
        }
        else if (ret_val == 0)
        {
            pArg->m_pObj->deleteClient(sock_clt);
			cout << "user " << socket_index
				<< " connection closed..." << endl;
        }
        else
        {
            pArg->m_pObj->deleteClient(sock_clt);
            cerr << "Failed to receive message from client " << socket_index
				<< "! Error code: " << ::GetLastError() << "\n";

            ::closesocket(sock_clt);
            return 1;
        }
    } while (ret_val > 0);

    pArg->m_pObj->deleteClient(sock_clt);

    //
    ret_val = ::shutdown(sock_clt, SD_SEND);
    if (ret_val == SOCKET_ERROR)
    {
        cerr << "Failed to shutdown the client " << socket_index 
			<< " socket! Error code: " << ::GetLastError() << "\n";
        ::closesocket(sock_clt);
        return 1;
    }

    return 0;
}
