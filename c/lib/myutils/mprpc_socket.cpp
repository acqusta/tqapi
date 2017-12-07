#include <assert.h>
#include <glog/logging.h>

#include "myutils/socketutils.h"
#include "myutils/misc.h"
#include "myutils/mprpc_socket.h"
#include "mprpc.h"
#include "snappy/snappy.h"

namespace mprpc {

    using namespace std;
    using namespace std::chrono;

    SocketMpRpcServer::SocketMpRpcServer(MpRpcServer* server)
        : m_server(server)
        , m_socket(INVALID_SOCKET)
        , m_should_exit(false)
        , m_connected(false)
    {
        m_main_thread = new thread(&SocketMpRpcServer::main_run, this);
    }

    SocketMpRpcServer::~SocketMpRpcServer()
    {
        m_should_exit = true;
        m_main_thread->join();
        delete m_main_thread;

        if (m_socket != INVALID_SOCKET)
            closesocket(m_socket);
    }

    bool SocketMpRpcServer::listen(const string& addr)
    {
        if (addr.empty() || m_addr.size()) return false;

        string ip;
        int port;
        if (!myutils::parse_addr(addr.c_str(), &ip, &port)){
            LOG(FATAL) << "Wrong addr format " << addr;
            return false;
        }

        SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == INVALID_SOCKET)
            return false;

        m_addr = addr;
        int v = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(v));

        sockaddr_in s_addr;
        memset(&s_addr, 0, sizeof(s_addr));
        s_addr.sin_family = AF_INET;
        s_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        s_addr.sin_port = htons(port);
        
        if (::bind(s, (sockaddr*)&s_addr, sizeof(s_addr)) != 0) {
            LOG(FATAL) << "Error binding socket, ec: " << WSAGetLastError();
            closesocket(s);
            return false;
        }

        int r = ::listen(s, 1);
        CHECK(!r) << "can't listen at " << addr;
        m_socket = s;

        return true;
   }

   void SocketMpRpcServer::close()
   {
   }

   void SocketMpRpcServer::main_run()
   {
       while (!m_should_exit) {
           if (m_socket == INVALID_SOCKET) {
               this_thread::sleep_for(milliseconds(100));
               continue;
           }

           fd_set rset, wset;
           FD_ZERO(&rset);
           FD_ZERO(&wset);

           SOCKET high_sock = m_socket;
           FD_SET(m_socket, &rset);
           FD_SET(m_socket, &wset);
           for (auto& e : m_client_map) {
               FD_SET(e.first, &rset);
               FD_SET(e.first, &wset);
               if (e.first > high_sock)
                   high_sock = e.first;
           }

           struct timeval tv;
           tv.tv_sec = 0;
           tv.tv_usec = 100 * 1000;

           int r = ::select(high_sock + 1, &rset, &wset, NULL, &tv);
           if (r == -1)
               continue;

           if (FD_ISSET(m_socket, &rset))
               do_accept();

           do_recv(&rset);
       }
    }

    void SocketMpRpcServer::do_accept()
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        socklen_t addr_len = sizeof(addr);

        SOCKET sock = ::accept(m_socket, (sockaddr*)&addr, &addr_len);
        if (sock == INVALID_SOCKET)
            return;

        myutils::set_socket_nonblock(m_socket);

        stringstream ss;
        ss << myutils::random();
        string id = ss.str();
        auto client = make_shared<SocketMpRpcServer_ClientConnection>(id, sock, this);
        m_client_map[sock] = client;
    }

    void SocketMpRpcServer::do_recv(fd_set* rset)
    {
        for (auto it = m_client_map.begin(); it != m_client_map.end(); ){
            if (!FD_ISSET(it->first, rset)) {
                it++;
                continue;
            }

            auto conn = it->second;
            if (conn->m_pkt_size) {
                int left_len = conn->m_pkt_size - conn->m_recv_size;
                int r = recv(conn->m_sock, (char*)conn->m_buf.c_str() + conn->m_recv_size, left_len, 0);
                if (r > 0) {
                    conn->m_recv_size += r;
                    if (conn->m_recv_size == conn->m_pkt_size) 
                        this->m_server->on_recv(conn, (const char*)conn->m_buf.c_str(), conn->m_pkt_size);
                    conn->m_pkt_size = 0;
                    conn->m_recv_size = 0;
                    it++;
                } else if (is_EWOURLDBLOCK(r)) {
                    it++;
                } else {
                    this->m_server->on_close(conn);
                    m_client_map.erase(it);
                }
            } else {
                int32_t pkt_size = 0;
                int r = recv(conn->m_sock, (char*)&pkt_size, 4, 0);
                if (r == 4) {
                    conn->m_recv_size  = 0;
                    conn->m_pkt_size = pkt_size;
                    conn->m_buf.resize(pkt_size);
                    it++;
                } else if (is_EWOURLDBLOCK(r)) {
                    it++;
                } else {
                    this->m_server->on_close(conn);
                    m_client_map.erase(it);
                }
            }
        }
    }

}
