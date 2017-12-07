#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <glog/logging.h>
#include "socket_connection.h"
#include "myutils/stringutils.h"

// On Windows the default value of FD_SETSIZE is 64.
#ifdef _WIN32
#define FD_SETSIZE 1024
#endif

#include <set>
#include <glog/logging.h>
#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#else
#  include <sys/socket.h>
#  include <netdb.h>
#  include <arpa/inet.h>
#  include <fcntl.h>
#endif
//#include "unicode.h"
#include "loop/MessageLoop.h"
#include "loop/RunLoop.h"

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

static SOCKET connect_socket(const char* ip_addr, int port)
{
    LOG(INFO) << "connect " << ip_addr << "," << port;
    
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

    int r = connect(s, (sockaddr*)&addr, sizeof(addr));
    if (r == 0 || !is_EWOURLDBLOCK(r)) {
        LOG(ERROR) << "connect error: " << ip_addr << "," << r << "," << strerror(errno) << "," << WSAGetLastError();
        closesocket(s);
        return INVALID_SOCKET;
    }

    LOG(ERROR) << "return " << s;
    return s;
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

static bool check_connect(SOCKET sock, int timeout_second) //, fd_set* rset, fd_set* wset)
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
            LOG(ERROR) << "RSET";
            return false;
        }
        else if (FD_ISSET(sock, &wset)) {
            LOG(ERROR) << "connected";
            return true;
        }
        else if (system_clock::now() - begin_time > seconds(timeout_second)) {
            LOG(ERROR) << "timeout";
            return false;
        }
    }
}


//static void check_recv(fd_set* rset)
//{
//    char buf[4096];
//    for (auto it = g_socksvr.turnel_list.begin(); it != g_socksvr.turnel_list.end();) {
//        auto t = *it;
//        if (!FD_ISSET(t->sock, rset)) {
//            it++;
//            continue;
//        }
//
//        auto r = recv(t->sock, buf, sizeof(buf), 0);
//        if (r > 0) {
//            VLOG(1) << "socks5: forward data to server " << t->id;
//            rpc::Body body;
//            auto ind = body.mutable_sock_msg()->mutable_data_ind();
//            ind->set_turnel_id(t->id);
//            ind->set_data(buf, r);
//            ind->set_seq(t->recv_seq++);
//            sysapi_send_ind(body);
//            it++;
//        }
//        else if (is_EWOURLDBLOCK(r)) {
//            it++;
//        }
//        else {
//            VLOG(1) << "socks5: remote disconnected";
//            close_turnel(t);
//            it = g_socksvr.turnel_list.erase(it);
//        }
//    }
//}

/**
* Socksvr 主线程
*/
//static void socksvr_run()
//{
//    fd_set rset, wset;
//    while (!g_should_exit) {
//        check_pending_msg_list();
//        int r = select(&rset, &wset);
//        if (r == -1) {
//            VLOG(1) << "select error: " << errno << "," << strerror(errno);
//            FD_ZERO(&rset);
//            FD_ZERO(&wset);
//        }
//
//        if (FD_ISSET(g_socksvr.cmd_sock_server, &rset)) {
//            char buf[1024];
//            recv(g_socksvr.cmd_sock_server, buf, 1024, 0);
//        }
//
//        check_connect(&rset, &wset);
//        check_recv(&rset);
//        check_dead_turnel();
//    }
//}

//// Linux & OSX can create pipe instead
//static void create_cmd_sock_pair()
//{
//    // 监听 cmd_sock_server, 建立连接后，关闭 cmd_sock，保留 cmd_sock_client, cmd_sock_server
//
//    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
//    CHECK(sock != INVALID_SOCKET);
//
//    int v = 1;
//    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(v));
//
//    sockaddr_in addr;


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
            LOG(ERROR) << "socket is " << m_socket;
        }

        fd_set rset, wset;
        int r = select(m_socket, &rset, &wset);
        if (r != -1) {
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

void SocketConnection::do_recv()
{
    if (m_pkg_size == 0) {
        int32_t i;
        int r = recv(m_socket, (char*)&i, 4, 0);
        if (r == 4) {
            m_pkg_size = i;
            if (m_pkg_size < 0 || m_pkg_size > 100 * 1024 * 1024) {
                closesocket(m_socket);
                m_socket = INVALID_SOCKET;
                return;
            }

            if (m_buf.size() < m_pkg_size) m_buf.resize(m_pkg_size);
            m_recv_size = 0;
        }
        else if (is_EWOURLDBLOCK(r)) {
            //return;
        }
        else {
            LOG(ERROR) << "recv error " << r;
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
        }
    }
    else {
        int r = recv(m_socket, (char*)m_buf.c_str() + m_recv_size, m_pkg_size - m_recv_size, 0);
        if (r > 0) {
            m_recv_size += r;
            if (m_recv_size == m_pkg_size) {
                this->m_callback->on_recv(m_buf.c_str(), m_pkg_size);
                m_recv_size = 0;
                m_pkg_size = 0;
            }
            else if (is_EWOURLDBLOCK(r)) {
                //return;
            }
            else {
                closesocket(m_socket);
                m_socket = INVALID_SOCKET;
            }
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

    m_pkg_size = 0;
    m_recv_size = 0;
    vector<string> ss;
    split(addr->c_str() + 6, ":", &ss);
    if (ss.size() != 2) return false;

    string ip = ss[0];
    int port = atoi(ss[1].c_str());

    LOG(INFO) << "connect to " << *addr;

    SOCKET sock = connect_socket(ip.c_str(), port);
    if (sock != INVALID_SOCKET) {
        if (check_connect(sock, 2))
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


void SocketConnection::send(const char* data, size_t size)
{
    unique_lock<mutex> lock(m_send_lock);
    
    if (m_socket != INVALID_SOCKET)
        ::send(m_socket, data, size, 0);
}

void SocketConnection::send(const string& data)
{
    send(data.c_str(), data.size());
}
