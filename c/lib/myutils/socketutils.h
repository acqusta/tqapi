#ifndef _MYUTILS_SOCKUTILS_H
#define _MYUTILS_SOCKUTILS_H

#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <glog/logging.h>
#include "socket_connection.h"
#include "myutils/stringutils.h"
#include <set>
#include <glog/logging.h>
#include <chrono>

// On Windows the default value of FD_SETSIZE is 64.
#ifdef _WIN32
# define FD_SETSIZE 1024
# include <Windows.h>
#else
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <fcntl.h>
#endif

#ifndef _WIN32

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
    static bool check_connect(SOCKET sock, int timeout_ms);
    
    static inline void set_socket_nonblock(SOCKET socket)//, bool nonblock)
    {
#ifdef _WIN32
        unsigned long nNonBlocking = 1;
        if (ioctlsocket(socket, FIONBIO, &nNonBlocking) == SOCKET_ERROR)
            LOG(FATAL) << "Unable to set nonblocking mode: " << WSAGetLastError();
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

    static SOCKET connect_socket(const char* ip_addr, int port, int timeout_ms)
    {
        sockaddr_in addr;
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

        if (check_connect(s, timeout_ms)){
            return s;
        } else {
            VLOG(1) << "connect error: " << ip_addr << "," << strerror(errno) << "," << WSAGetLastError();
            closesocket(s);
            return INVALID_SOCKET;
        }
    }

    static int select(SOCKET socket, fd_set* rset, fd_set* wset)
    {
        FD_ZERO(rset);
        FD_ZERO(wset);
        
        SOCKET high_sock = socket;
        FD_SET(socket, rset);
        FD_SET(socket, wset);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;

        return ::select(high_sock + 1, rset, wset, NULL, &tv);
    }

    static bool check_connect(SOCKET sock, int timeout_ms)
    {
        auto begin_time = system_clock::now();
        
        fd_set rset, wset;
        while (true) {
            FD_ZERO(&rset);
            FD_ZERO(&wset);
            SOCKET high_sock = sock;
            FD_SET(sock, &rset);
            FD_SET(sock, &wset);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100 * 1000;

            int r = ::select(high_sock + 1, &rset, &wset, NULL, &tv);
            if (r == -1) {
                FD_ZERO(&rset);
                FD_ZERO(&wset);
            }
            auto now = system_clock::now();
            if (FD_ISSET(sock, &rset)) {
                return false;
            }
            else if (FD_ISSET(sock, &wset)) {
                return true;
            }
            else if (now - begin_time > milliseconds(timeout_ms)) {
                return false;
            }
        }
    }

    static bool parse_addr(const char* addr, string* ip, int* port)
    {
        if (memcmp(addr, "tcp://", 6)!=0) return false;
        const char* p = addr + 6;
        const char* p2 = strrchr(p, ':');
        if (p2) {
            *ip = string(p, p2-p);
            *port = atoi(p2+1);
            return true;
        } else {
            return false;
        }
    }
}

#endif
