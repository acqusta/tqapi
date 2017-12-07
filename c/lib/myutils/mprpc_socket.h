#ifndef _MPRPC_SOCKET_H
#define _MPRPC_SOCKET_H

#include <string>
#include <unordered_map>
#include "myutils/mprpc.h"
#include "socket_connection.h"
#include "myutils/socketutils.h"

namespace mprpc {

    using namespace std;
    using namespace std::chrono;

    class SocketMpRpcServer_ClientConnection;

    class SocketMpRpcServer {
        friend class SocketMpRpcServer_ClientConnection;
    public:

        SocketMpRpcServer(MpRpcServer* server);

        virtual ~SocketMpRpcServer();

        bool listen(const std::string& addr);

        void close();

    private:
        void main_run();
        void do_connect();
        void do_recv(fd_set* rset);
        void do_send_heartbeat();
        void do_listen();
        void do_accept();

        //bool send(const string& id, const char* data, size_t size);

    private:
        MpRpcServer*        m_server;
        volatile SOCKET     m_socket;
        string              m_addr;
        mutex               m_send_lock;
        thread*             m_main_thread;
        volatile bool       m_should_exit;
        bool                m_connected;
        unordered_map<SOCKET, shared_ptr<SocketMpRpcServer_ClientConnection>> m_client_map;
    };

    class SocketMpRpcServer_ClientConnection : public ClientConnection {
        friend class SocketMpRpcServer;
    public:

        SocketMpRpcServer_ClientConnection(const string& id, SOCKET sock, SocketMpRpcServer* server)
            : m_id(id)
            , m_sock(sock)
            , m_server(server)
            , m_pkt_size(0)
            , m_recv_size(0)
        {
        }

        virtual ~SocketMpRpcServer_ClientConnection() {
            if (m_sock != INVALID_SOCKET)
                closesocket(m_sock);
        }
        virtual string id() override { return m_id; }

        virtual bool send(const char* data, size_t size) override
        {
            // FIXME:
            int32_t pkt_size = size;
            int r = ::send(m_sock, (const char*)&pkt_size, 4, 0);
            if (r!=4) {
                closesocket(m_sock);
                m_sock = INVALID_SOCKET;
                return false;
            }

            r = ::send(m_sock, data, size, 0);
            if (r!=4) {
                closesocket(m_sock);
                m_sock = INVALID_SOCKET;
                return false;
            }
        }

    private:
        string m_id;
        SOCKET m_sock;
        SocketMpRpcServer* m_server;
        int32_t m_pkt_size;
        int32_t m_recv_size;
        string  m_buf;
    };
}

#endif
