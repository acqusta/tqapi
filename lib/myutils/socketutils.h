#ifndef _MYUTILS_SOCKUTILS_H
#define _MYUTILS_SOCKUTILS_H

//#define _WINSOCKAPI_

#include <stdint.h>
#include <string>

#ifdef _WIN32
# include <winsock2.h>
# include <windows.h>
typedef int socklen_t;
#else
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <unistd.h>

typedef int SOCKET;

#ifndef INVALID_SOCKET
#  define INVALID_SOCKET -1
#endif

#ifndef closesocket
#  define closesocket ::close
#endif

#define WSAGetLastError() errno

#endif

#ifdef _WIN32
#  define is_EWOURLDBLOCK(r)   (r == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
#elif defined(__linux__)
#  define is_EWOURLDBLOCK(r)   (r == -1 &&  (errno == EAGAIN || errno == EINPROGRESS))
#elif defined(__APPLE__)
#  define is_EWOURLDBLOCK(r)   (r == -1 &&  (errno == EAGAIN || errno == EINPROGRESS))
#else
#  error "tbd: is_EWOURLDBLOCK"
#endif

namespace myutils {

    using namespace std;
    bool check_connect(SOCKET sock, int timeout_ms);

#ifdef _WIN32
    void init_winsock2();
#endif

    void set_socket_nonblock(SOCKET socket, bool nonblock);

    uint32_t resolve_name(const string& name);

    SOCKET connect_socket(const char* ip_addr, int port);

    int select(SOCKET socket, fd_set* rset, fd_set* wset);

    bool check_connect(SOCKET sock, int timeout_second);

    bool parse_addr(const char* addr, string* ip, int* port);

    bool create_cmd_sock_pair(SOCKET* server, SOCKET* client);
}

#endif
