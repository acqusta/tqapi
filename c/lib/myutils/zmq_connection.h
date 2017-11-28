#ifndef _ZMQCONNECTION_H
#define _ZMQCONNECTION_H

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include "zmq.hpp"

using namespace std;
using namespace std::chrono;


class ZmqConnection_Callback {
public:
    virtual void on_recv(const char* data, size_t size) = 0;
    virtual void on_idle() = 0;
};

class ZmqConnection {

public:

    ZmqConnection();

    virtual ~ZmqConnection();

    void set_key(const char* server_key, const char* pub_key, const char* sec_key) {
        m_server_key = server_key;
        m_pub_key = pub_key;
        m_sec_key = sec_key;
    }

    void set_identity(const string& identity) {
        m_identity = identity;
    }

    bool connect(const std::string& addr, ZmqConnection_Callback* callback);

    void reconnect();

    void close();

    void send(const char* data, size_t size);
    void send(const string& data);

private:
    void main_run();
    void do_send(const char* buf, size_t size);
    void do_connect();
    void do_recv();
    void do_send_heartbeat();

    void asyncall(function<void()>);

private:
    string                      m_addr;
    mutex                       m_send_lock;
    zmq::context_t              m_zmq_ctx;
    zmq::socket_t*              m_push_sock;
    zmq::socket_t*              m_pull_sock;
    zmq::socket_t*              m_remote_sock;
    ZmqConnection_Callback*     m_callback;
    thread*                     m_main_thread;
    volatile bool               m_should_exit;
    bool                        m_connected;
    string                      m_server_key;
    string                      m_pub_key;
    string                      m_sec_key;
    string                      m_identity;
};


#endif
