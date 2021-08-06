#include "socketclient.h"

bool initWsaData();

DWORD WINAPI receiveMessageThread(LPVOID lpParameter);


SocketClient::SocketClient()
{
  if (init() == false) {
	cout << "client init fail" << endl;
  } else {
	cout << "client init success" << endl;
    string s = "send message";
    sendMessage(s);
  }



  return;
}

bool SocketClient::init()
{

  if (!initWsaData()) {
    initFlag = false;
    return false;
  }

  //填充服务端信息
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(8888);

  //创建套接字
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (::connect(server_socket, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
    cout << "fail to connect server" << endl;
    WSACleanup();
    initFlag = false;
    return false;
  }

  // use a thread to receive from server
  h_thread = ::CreateThread(nullptr,
                            0,
                            receiveMessageThread,
                            (LPVOID)server_socket,
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

bool SocketClient::sendMessage(string msg)
{
  if (initFlag != true) {
    cout << "send message fail: not init" << endl;
    return false;
  }

  const char* c = msg.c_str();
  if (send(server_socket, c, msg.length(), 0) == SOCKET_ERROR) {
    cout << "fail to send message to server:  socket = " << server_socket << endl;
    return false;
  }
  return true;
}

// do something to handle message
int SocketClient::handleMessage(string msg)
{
  cout << "receive success: " << msg << endl;
  return 0;
}



// a thread used to receive message from server
DWORD WINAPI receiveMessageThread(LPVOID lpParameter) {

  ThreadArg* pArg = (ThreadArg*)lpParameter;
  SOCKET server_socket = pArg->m_socket;

  char recv_buf[MAX_LENGTH];

  while (1) {
    int recv_len = recv(server_socket, recv_buf, MAX_LENGTH, 0);
    if (recv_len < 0) {
      cout << "receive fail";
      return 1;

    } else {
      string s = recv_buf;
      pArg->m_pObj->handleMessage(s);
    }
  }

  return 0;
}

bool initWsaData() {
  //初始化套接字库
  WORD w_req = MAKEWORD(2, 2);//版本号
  WSADATA wsadata;
  int err;
  err = WSAStartup(w_req, &wsadata);
  if (err != 0) {
    cout << "init WSA data fail" << endl;
    return false;
  }

  //检测版本号
  if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
    cout << "WSA data version wrong" << endl;
    WSACleanup();
    return false;
  }
  else {
    return true;
  }

}


