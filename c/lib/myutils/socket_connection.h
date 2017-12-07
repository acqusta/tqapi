#ifndef _SOCK_CONNECTION_H
#define _SOCK_CONNECTION_H

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include "connection.h"

#ifndef _WIN32

typedef int SOCKET;

#define INVALID_SOCKET -1
#define closesocket ::close
#define WSAGetLastError() errno

#else

typedef int socklen_t;

#endif

using namespace std;
using namespace std::chrono;

class SocketConnection : public Connection {

public:

    SocketConnection();

    virtual ~SocketConnection();

    virtual bool connect(const std::string& addr, Connection_Callback* callback) override;
    virtual void reconnect() override;
    virtual void close() override;
    virtual void send(const char* data, size_t size) override;
    virtual void send(const std::string& data) override;

private:
    void main_run();
    void do_send(const char* buf, size_t size);
    bool do_connect();
    void do_recv();
    void do_send_heartbeat();

    void asyncall(function<void()>);

private:
    shared_ptr<string>          m_addr;
    mutex                       m_send_lock;
    SOCKET                      m_socket;
    Connection_Callback*        m_callback;
    thread*                     m_main_thread;
    volatile bool               m_should_exit;
    bool                        m_connected;
    string                      m_buf;
    int32_t                     m_pkg_size;
    int32_t                     m_recv_size;
};


#endif
