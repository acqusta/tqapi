#include "socketutils.h"

#include <assert.h>
#include <chrono>
#include <stdint.h>
#include <string.h>
#include <string>
#ifdef __linux__
# include <signal.h>
#endif

#include "myutils/stringutils.h"

namespace myutils {

    using namespace std;
    using namespace std::chrono;

#ifdef _WIN32

    class Winsock2Initialization {
    public:
        Winsock2Initialization() {
            init_winsock2();
        }
    };

    static Winsock2Initialization g_sock2_init;

    void init_winsock2()
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

    void set_socket_nonblock(SOCKET socket, bool nonblock)
    {
    #ifdef _WIN32
        unsigned long nNonBlocking = nonblock ? 1 : 0;
        if (ioctlsocket(socket, FIONBIO, &nNonBlocking) == SOCKET_ERROR)
            assert(false);
    #else
        int flags = fcntl(socket, F_GETFL, 0);
	if (nonblock)
            flags |= O_NONBLOCK;
        else
            flags &= ~O_NONBLOCK;  
        fcntl(socket, F_SETFL, flags);
    #endif
    }


    /*
    return INADDR_NONE if failed
    FIXME: may block system!
    */
    uint32_t resolve_name(const string& name)
    {
        hostent* h = gethostbyname(name.c_str());
        if (!h) return INADDR_NONE;

        if (h->h_addrtype != AF_INET) return 0;
        // return first addr
        return *(uint32_t*)h->h_addr;
    }

    SOCKET connect_socket(const char* ip_or_host, int port)
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip_or_host);
        if (addr.sin_addr.s_addr == INADDR_NONE) {
            addr.sin_addr.s_addr = resolve_name(ip_or_host);
            if (addr.sin_addr.s_addr == INADDR_NONE)
                return INVALID_SOCKET;
        }

        SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET)
            return false;

#ifdef _WIN32
        int v = 2000;
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&v, sizeof(v));
        setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&v, sizeof(v));
#else
        struct timeval timeout = { 2, 0 * 1000 };
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
#endif

        set_socket_nonblock(s, true);

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

    int select(SOCKET socket, fd_set* rset, fd_set* wset)
    {
        if (rset) FD_ZERO(rset);
        if (wset) FD_ZERO(wset);

        if (rset) FD_SET(socket, rset);
        if (wset) FD_SET(socket, wset);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500 * 1000;

        int high_sock = (int)socket;
        return ::select(high_sock + 1, rset, wset, NULL, &tv);
    }

    bool check_connect(SOCKET sock, int timeout_second) //, fd_set* rset, fd_set* wset)
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


    bool parse_addr(const char* addr, string* ip, int* port)
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

    // 监听 cmd_sock_server, 建立连接后，关闭 cmd_sock，保留 cmd_sock_client, cmd_sock_server
    bool create_cmd_sock_pair(SOCKET* server, SOCKET* client)
    {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) return false;

        int v = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(v));

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = 0;

        if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) != 0) {
            //LOG(FATAL) << "Error binding socket, ec: " << WSAGetLastError();
            closesocket(sock);
            return false;
        }

        listen(sock, 1);
        socklen_t addr_len = sizeof(addr);
        getsockname(sock, (sockaddr*)&addr, &addr_len);
        ////LOG(INFO) << "socks turnel server listen at " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port);

        string ip = inet_ntoa(addr.sin_addr);
        int port = ntohs(addr.sin_port);
        *client = myutils::connect_socket(ip.c_str(), port);
        //CHECK(*client != INVALID_SOCKET);
        if (!*client) {
            closesocket(sock);
            return false;
        }

        *server = accept(sock, (sockaddr*)&addr, &addr_len);
        if (!*server) {
            closesocket(sock);
            closesocket(*client);
            return false;
        }

        myutils::set_socket_nonblock(*server, true);
        closesocket(sock);

        return true;
    }
}
