#ifndef _MYUTILS_SOCKUTILS_H
#define _MYUTILS_SOCKUTILS_H

#define _WINSOCKAPI_

#include <assert.h>
#include <chrono>
#include <stdint.h>
#include <string>
#include <string.h>
#include "myutils/stringutils.h"

// On Windows the default value of FD_SETSIZE is 64.
#ifdef _WIN32
# define FD_SETSIZE 1024
# include <WinSock2.h>
#else
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <fcntl.h>
#endif

#ifndef _WIN32

#include <unistd.h>

typedef int SOCKET;
#define INVALID_SOCKET -1
#define closesocket ::close
#define WSAGetLastError() errno

#else

typedef int socklen_t;

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
    using namespace std::chrono;

    static bool check_connect(SOCKET sock, int timeout_ms);

#ifdef _WIN32
    static inline void init_winsock2()
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;

        /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
        wVersionRequested = MAKEWORD(2, 2);

        err = WSAStartup(wVersionRequested, &wsaData);
        if (err != 0) {
            /* Tell the user that we could not find a usable */
            /* Winsock DLL.                                  */
            printf("WSAStartup failed with error: %d\n", err);
        }
    }
#endif

    static inline void set_socket_nonblock(SOCKET socket)//, bool nonblock)
    {
    #ifdef _WIN32
        unsigned long nNonBlocking = 1;
        if (ioctlsocket(socket, FIONBIO, &nNonBlocking) == SOCKET_ERROR)
            assert(false);
    #else
        int flags = fcntl(socket, F_GETFL, 0);
        flags |= O_NONBLOCK;
        fcntl(socket, F_SETFL, flags);
    #endif
    }


    /*
    return INADDR_NONE if failed
    FIXME: may block system!
    */
    static uint32_t resolve_name(const string& name)
    {
        hostent* h = gethostbyname(name.c_str());
        if (!h) return INADDR_NONE;

        if (h->h_addrtype != AF_INET) return 0;
        // return first addr
        return *(uint32_t*)h->h_addr;
    }

    static SOCKET connect_socket(const char* ip_addr, int port)
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip_addr);
        if (addr.sin_addr.s_addr == INADDR_NONE) {
            addr.sin_addr.s_addr = resolve_name(ip_addr);
            if (addr.sin_addr.s_addr == INADDR_NONE)
                return INVALID_SOCKET;
        }

        SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET)
            return false;

        set_socket_nonblock(s);

        int r = connect(s, (sockaddr*)&addr, sizeof(addr));
        if (r == 0 || is_EWOURLDBLOCK(r)) {
            return s;
        } 
        else {
            //LOG(ERROR) << "connect error: " << ip_addr << "," << r << "," << strerror(errno) << "," << WSAGetLastError();
            closesocket(s);
            return INVALID_SOCKET;
        }

        return s;
    }

    static int select(SOCKET socket, fd_set* rset, fd_set* wset)
    {
        FD_ZERO(rset);
        FD_ZERO(wset);

        FD_SET(socket, rset);
        FD_SET(socket, wset);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;

        int high_sock = (int)socket;
        return ::select(high_sock + 1, rset, wset, NULL, &tv);
    }

    static bool check_connect(SOCKET sock, int timeout_second) //, fd_set* rset, fd_set* wset)
    {
        auto begin_time = system_clock::now();

        while (system_clock::now() - begin_time < seconds(timeout_second)) {
            SOCKET high_sock = sock;

            fd_set rset, wset;
            int r = select(high_sock, &rset, &wset);
            if (r == -1 || r == 0 || is_EWOURLDBLOCK(r)) {
                continue;
            }
            else if (FD_ISSET(sock, &rset)) {
                int err = 0;
                socklen_t len = sizeof(err);
                int ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&err, &len);
                if (ret != 0) err = 1;

                return err == 0;
            }
            else if (FD_ISSET(sock, &wset)) {
                return true;
            }
        }

        return false;
    }


    static bool parse_addr(const char* addr, string* ip, int* port)
    {
        if (memcmp(addr, "tcp://", 6) != 0) return false;
        const char* p = addr + 6;
        const char* p2 = strrchr(p, ':');
        if (p2) {
            *ip = string(p, p2 - p);
            *port = atoi(p2 + 1);
            return true;
        }
        else {
            return false;
        }
    }
}

#endif
