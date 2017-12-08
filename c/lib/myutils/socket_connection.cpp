#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include "myutils/socket_connection.h"
#include "myutils/socketutils.h"
#include "myutils/stringutils.h"



SocketConnection::SocketConnection()
    : m_socket(INVALID_SOCKET)
    , m_callback(nullptr)
    , m_should_exit(false)
    , m_connected(false)
{
    m_main_thread = new thread(&SocketConnection::main_run, this);
}

SocketConnection::~SocketConnection()
{
    m_should_exit = true;

    m_main_thread->join();

    delete m_main_thread;

    if (m_socket != INVALID_SOCKET)
        closesocket(m_socket);
}

void SocketConnection::main_run()
{
    auto idle_time = system_clock::now();
    while (!m_should_exit) {

        if (m_socket == INVALID_SOCKET) {
            if (!do_connect()) {
                this_thread::sleep_for(seconds(1));
                continue;
            }
        }

        fd_set rset;
        int r = myutils::select(m_socket, &rset, nullptr);
        if (r > 0) {
            if (FD_ISSET(m_socket, &rset))
                do_recv();
        }
        auto now = system_clock::now();
        if (now < idle_time || now - idle_time > milliseconds(200)){
            idle_time = now;
            m_callback->on_idle();
        }
    }
}

void SocketConnection::do_close(const char* reason)
{
    if (reason) {
        //LOG(ERROR) << "close socket: " << reason << "," << WSAGetLastError();
        std::cerr << "close socket: " << reason << "," << WSAGetLastError();
    }
    if (m_socket == INVALID_SOCKET) return;

    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
    m_recv_size = 0;
    m_pkt_size = 0;
}

void SocketConnection::do_recv()
{
    if (m_pkt_size == 0) {
        int32_t pkt_size;
        int r = recv(m_socket, (char*)&pkt_size, 4, 0);
        if (r == 4) {
            if (pkt_size < 0 || pkt_size > 100 * 1024 * 1024) {
                do_close("wrong pkt_size");
                return;
            }

            m_recv_size = 0;
            m_pkt_size = pkt_size;

            if (m_buf.size() < pkt_size) m_buf.resize(pkt_size);
        }
        else if (is_EWOURLDBLOCK(r)) {
            //return;
        }
        else {
            do_close("recv error");
        }
    }
    else {
        int r = recv(m_socket, (char*)m_buf.c_str() + m_recv_size, m_pkt_size - m_recv_size, 0);
        if (r > 0) {
            m_recv_size += r;
            if (m_recv_size == m_pkt_size) {
                this->m_callback->on_recv(m_buf.c_str(), m_pkt_size);
                m_recv_size = 0;
                m_pkt_size = 0;
            }
        }
        else if (is_EWOURLDBLOCK(r)) {
            //return;
        }
        else {
            do_close("recv error");
        }
    }
}

bool SocketConnection::connect(const string& addr, Connection_Callback* callback)
{
    m_callback = callback;
    m_addr = make_shared<string>(addr);

    return true;
}

void SocketConnection::reconnect()
{
}

bool SocketConnection::do_connect() 
{
    auto addr = m_addr;
    if (!addr) return false;

    m_pkt_size = 0;
    m_recv_size = 0;
    vector<string> ss;
    split(addr->c_str() + 6, ":", &ss);
    if (ss.size() != 2) return false;

    string ip = ss[0];
    int port = atoi(ss[1].c_str());

    SOCKET sock = myutils::connect_socket(ip.c_str(), port);
    if (sock != INVALID_SOCKET) {
        if (myutils::check_connect(sock, 2))
            m_socket = sock;
        else
            closesocket(sock);
    }

    return m_socket != INVALID_SOCKET;
}

void SocketConnection::close()
{
    m_should_exit = true;
}

static inline bool try_send(SOCKET sock, const char* data, size_t size)
{
    for (int i = 0; i < 5; i++) {
        int r = ::send(sock, data, size, 0);
        if (r == size) return true;
        if (r == -1 && is_EWOURLDBLOCK(r)) continue;
        break;
    }
    return false;
}

void SocketConnection::send(const char* data, size_t size)
{
    unique_lock<mutex> lock(m_send_lock);
    
    if (m_socket != INVALID_SOCKET) {

        int32_t len = (int32_t)size;
        if (!try_send(m_socket, (const char*)&len, 4) ||
            !try_send(m_socket, data, size))
        {
            do_close("send error");
        }
    }
}

void SocketConnection::send(const string& data)
{
    send(data.c_str(), data.size());
}
