#include "socketserver.h"

int main()
{
    SocketServer svr;
    svr.WaitForClient();
    return 0;
}
