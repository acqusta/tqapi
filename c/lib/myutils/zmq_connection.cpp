#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <glog/logging.h>
#include "zmq_connection.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif


using namespace std;

ZmqConnection::ZmqConnection() :
    m_remote_sock(nullptr),
    m_callback(nullptr),
    //m_cur_callid()
    m_should_exit(false),
    m_connected(false)
{
    m_pull_sock = new zmq::socket_t(m_zmq_ctx, ZMQ_PULL);
    m_pull_sock->bind("inproc://pull_sock");
    m_push_sock = new zmq::socket_t(m_zmq_ctx, ZMQ_PUSH);
    m_push_sock->connect("inproc://pull_sock");

    m_main_thread = new thread(&ZmqConnection::main_run, this);
}

ZmqConnection::~ZmqConnection()
{
    m_should_exit = true;

    m_main_thread->join();

    delete m_main_thread;

    delete m_pull_sock;
    delete m_push_sock;
    if (m_remote_sock) delete m_remote_sock;
}

void ZmqConnection::main_run()
{
    zmq_pollitem_t items[2];
    memset(items, 0, sizeof(items));
    int items_count = 1;
    items[0].socket = (void*)*m_pull_sock;
    items[0].events = ZMQ_POLLIN;

    auto idle_time = system_clock::now();
    while (!m_should_exit) {

        try {
            if (m_remote_sock) {
                items[1].socket = (void*)*m_remote_sock;
                items[1].events = ZMQ_POLLIN;
                items_count = 2;
            }

            zmq::poll(items, items_count, 100);

            if (items[0].revents & ZMQ_POLLIN) {
                zmq::message_t cmd;
                zmq::message_t data;
                if (m_pull_sock->recv(&cmd)) {
                    if (cmd.more())
                        m_pull_sock->recv(&data);

                    const char* p = (const char*)cmd.data();
                    switch (p[0]){
                    case 'S':   do_send((const char*)data.data(), data.size());     break;
                    case 'C':   do_connect(); break;
                    case 'R':   do_connect(); break;
                    }
                }
            }

            if (items_count == 2)
                if (items[1].revents & ZMQ_POLLIN)  do_recv();
            
            auto now = system_clock::now();
            if (now < idle_time || now - idle_time > seconds(1)){
                idle_time = now;
                m_callback->on_idle();
            }
        }
        catch (std::exception& e) {
            LOG(ERROR) << "main_run catch exception: " << e.what();
        }
    }

}

void ZmqConnection::do_recv()
{
    try {
        zmq::message_t msg;
        if (!m_remote_sock->recv(&msg, ZMQ_DONTWAIT)) return;

        if (m_callback)
            m_callback->on_recv((const char*)msg.data(), msg.size());
    }
    catch (exception& e) {
        std::cerr << e.what() << endl;
    }
}

bool ZmqConnection::connect(const string& addr, ZmqConnection_Callback* callback)
{
    if (addr.empty()) return false;
    m_addr = addr;
    m_callback = callback;

    unique_lock<mutex> lock(m_send_lock);
    m_push_sock->send("C", 1);
    return true;
}

void ZmqConnection::reconnect()
{
    unique_lock<mutex> lock(m_send_lock);
    m_push_sock->send("R", 1);
}

void ZmqConnection::do_connect() 
{
    assert(!m_addr.empty());

    //LOG(INFO) << "Connect to " << m_addr;
    if (m_remote_sock != nullptr) delete m_remote_sock;

    m_remote_sock = new zmq::socket_t(m_zmq_ctx, ZMQ_DEALER);
    m_remote_sock->setsockopt(ZMQ_RCVTIMEO, 2000);
    m_remote_sock->setsockopt(ZMQ_SNDTIMEO, 2000);
    m_remote_sock->setsockopt(ZMQ_LINGER, 0);

    if (!m_pub_key.empty()) {
        uint8_t sec_key[32];
        uint8_t pub_key[32];
    
        assert(m_sec_key.size() == 40);
        assert(m_pub_key.size() == 40);

        zmq_z85_decode(sec_key, m_sec_key.c_str());
        zmq_z85_decode(pub_key, m_pub_key.c_str());

        m_remote_sock->setsockopt(ZMQ_CURVE_SECRETKEY, sec_key, 32);
        m_remote_sock->setsockopt(ZMQ_CURVE_PUBLICKEY, pub_key, 32);
    }

    if (!m_server_key.empty()) {
        assert(m_server_key.size() == 40);
        m_remote_sock->setsockopt(ZMQ_CURVE_SERVERKEY, m_server_key.c_str(), 40);
    }

    //stringstream ss;
    //ss << my_rand() << "$" << my_rand();
    //string id = ss.str();

    if (!m_identity.empty())
        m_remote_sock->setsockopt(ZMQ_IDENTITY, m_identity.c_str(), m_identity.size());

    m_remote_sock->connect(m_addr);
}

void ZmqConnection::close()
{
    m_should_exit = true;
}


void ZmqConnection::send(const char* data, size_t size)
{
    unique_lock<mutex> lock(m_send_lock);
    m_push_sock->send("S", 1, ZMQ_SNDMORE);
    m_push_sock->send(data, size);
}

void ZmqConnection::send(const string& data)
{
    send(data.c_str(), data.size());
}

void ZmqConnection::do_send(const char* buf, size_t size)
{
    //LOG(INFO) << "Send " << string(buf, size);

    assert(m_remote_sock);
    try {
        m_remote_sock->send(buf, size);
    }
    catch (exception& e) {
        cerr << "do_send failed: " << e.what() << endl;
    }
}

