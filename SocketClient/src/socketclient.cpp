#include "socketclient.h"



DWORD WINAPI receiveMessageThread(LPVOID lpParameter);


SocketClient::SocketClient(bool start_after_init,
	bool block_to_wait_enter_instruct)
{
	m_ip = DEFAULT_SERVER_IP;
	m_port = DEFAULT_PORT;

	initFlag = false;

	if (start_after_init) {
		if (start(block_to_wait_enter_instruct)) {
			cout << "client init success" << endl;
		}
		else {
			cout << "client init fail" << endl;
		}
	}


}

bool SocketClient::init()
{
	// if had init, return
	if (initFlag) {
		return true;
	}

	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "init WSA data fail" << endl;

		initFlag = false;
		return false;
	}

	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "WSA data version wrong" << endl;
		WSACleanup();

		initFlag = false;
		return false;
	}

	//填充服务端信息
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = inet_addr(m_ip.c_str());
	server_addr.sin_port = htons(m_port);

	//创建套接字
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (::connect(server_socket, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "fail to connect server" << endl;
		WSACleanup();

		initFlag = false;
		return false;
	}


	// use a thread to receive from server
	ThreadArg* pArg = new ThreadArg(this, server_socket);
	h_thread = ::CreateThread(nullptr,
		0,
		receiveMessageThread,
		(LPVOID)pArg,
		0,
		nullptr);

	if (h_thread == NULL) {
		cerr << "Failed to create a new thread!Error code: " << ::WSAGetLastError() << "\n";
		::WSACleanup();

		initFlag = false;
		return false;
	}
	::CloseHandle(h_thread);


	initFlag = true;
	return initFlag;
}

bool SocketClient::start(bool block_to_wait_enter_instruct) {

	if (init()) {
		if (block_to_wait_enter_instruct) {
			while (1) {
				string str;
				cout << "enter instruct: ";
				cin >> str;
				if (str == "exit") {
					exit(0);
				}
				sendMessage(str);
			}
		}
		else {
			return true;
		}
	}
	else {
		return false;
	}

	return true;

}

bool SocketClient::close()
{
	//关闭套接字
	closesocket(server_socket);

	//释放DLL资源
	WSACleanup();

	return true;
}

bool SocketClient::setIpAndPort(string ip, int port)
{
	m_ip = ip;
	m_port = port;
	return init();
}

bool SocketClient::sendMessage(string msg)
{
	if (!initFlag) {
		cout << "send message fail: not init" << endl;
		return false;
	}

	const char* c = msg.c_str();
	if (send(server_socket, c, MSG_BUF_SIZE, 0) == SOCKET_ERROR) {
		cout << "fail to send message to server: server_socket = " << server_socket << ", error: " << WSAGetLastError() << endl;
		return false;
	}
	return true;
}


// do something to handle message
int SocketClient::handleMessage(string msg)
{

	return 0;
}



// a thread used to receive message from server
DWORD WINAPI receiveMessageThread(LPVOID lpParameter) {

	ThreadArg* pArg = (ThreadArg*)lpParameter;
	int server_socket = pArg->m_socket;

	char recv_buf[MSG_BUF_SIZE];

	while (1) {
		int recv_len = recv(server_socket, recv_buf, MSG_BUF_SIZE, 0);
		if (recv_len < 0) {
			cout << "receive fail";
			return 1;

		}
		else {
			string s = recv_buf;
			//      cout << "receive success";
			pArg->m_pObj->handleMessage(s);
		}
	}

	return 0;
}


