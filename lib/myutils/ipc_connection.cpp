#include <assert.h>
#include <algorithm>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <string.h>
#include <time.h>
#ifndef _WIN32
# include <sys/time.h>
#endif
#include "myutils/misc.h"
#include "myutils/ipc_connection.h"
#include "myutils/stringutils.h"
#include "myutils/timeutils.h"
#include "myutils/socketutils.h"

using namespace myutils;

IpcConnection::IpcConnection()
    : m_callback(nullptr)
    , m_should_exit(false)
    , m_connected(false)
    , m_my_id(0)
    , m_my_shmem(nullptr)
    , m_svr_shmem(nullptr)
    , m_recv_queue(nullptr)
    , m_send_queue(nullptr)
    , m_recv_thread(nullptr)
    , m_slot_info(nullptr)
    , m_slot(nullptr)
    , m_sem_send(nullptr)
    , m_sem_recv(nullptr)
    , m_socket(INVALID_SOCKET)
{
    m_msg_loop.post_delayed_task(bind(&IpcConnection::check_connection, this),  500);
    m_msg_loop.post_delayed_task(bind(&IpcConnection::on_idle_timer,    this), 1000);
}

IpcConnection::~IpcConnection()
{
    close();
}

void IpcConnection::recv_run()
{
    while (!m_should_exit) {
        if (m_slot->client_id != m_my_id)  break;
        switch (m_sem_recv->timed_wait(100)) {
        case 1:
            //msg_loop().post_task([this]() {
                do_recv(); 
            //});
            break;
        case 0:
            break;
        default:
            break;
        }
    }
    
    msg_loop().post_task([this]() {
        if (m_connected) {
            set_conn_stat(false);
            if (m_callback) m_callback->on_conn_status(false);
            do_connect();
        }
    });
}


void IpcConnection::do_recv()
{
    if (!m_recv_queue) return;

    const char* data;
    size_t size;

    // at most read 10 msgs in one loop
    for (int i = 0; i < 10; i++) {
        if (m_recv_queue->poll(&data, &size) ) {
            if (m_callback)
                m_callback->on_recv(data, size);
            m_recv_queue->pop();
        }
        else {
            break;
        }
    }
}

bool IpcConnection::connect(const string& addr, Connection_Callback* callback)
{
    mutex mtx;
    condition_variable cv;
    unique_lock<mutex> lock(mtx);

    m_msg_loop.post_task([this, &mtx, &cv, addr, callback]() {
        m_addr = addr;
        m_callback = callback;
        do_connect();
        unique_lock<mutex> lock(mtx);
        cv.notify_all();
    });

    cv.wait(lock);

    return m_connected;
}

void IpcConnection::reconnect()
{
    m_msg_loop.post_task([this] {
        set_conn_stat(false);
        do_connect();
    });
}


void IpcConnection::clear_data()
{
    unique_lock<mutex> lock(m_send_mtx);

    if (m_recv_thread) {
        m_should_exit = true;
        m_recv_thread->join();
        delete m_recv_thread;
        m_recv_thread = nullptr;
    }

    if (m_my_shmem) {
        delete m_my_shmem;
        m_my_shmem = nullptr;
    }

    if (m_svr_shmem) {
        delete m_svr_shmem;
        m_svr_shmem = nullptr;
    }

    m_send_queue = nullptr;
    m_recv_queue = nullptr;
    m_slot_info = nullptr;
    m_slot = nullptr;

    if (m_sem_recv) {
        delete m_sem_recv;
        m_sem_recv = nullptr;
    }

    if (m_sem_send) {
        delete m_sem_send;
        m_sem_send = nullptr;
    }

    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

void IpcConnection::check_connection()
{
    uint64_t my_id;
    int r = ::recv(m_socket, (char*)&my_id, sizeof(my_id), 0);
    m_connected = r > 0 || is_EWOURLDBLOCK(r);

    if (!m_connected) do_connect();

    m_msg_loop.post_delayed_task(bind(&IpcConnection::check_connection, this), 500);
}

void IpcConnection::on_idle_timer()
{
    m_msg_loop.post_delayed_task(bind(&IpcConnection::on_idle_timer, this), 1000);
    if (m_callback)
        m_callback->on_idle();
}

bool IpcConnection::do_connect() 
{
    if (m_connected) return true;

    if (m_addr.empty()) return false;

    if (system_clock::now() - m_last_connect_time < milliseconds(200))
        return false;

    m_last_connect_time = system_clock::now();

    clear_data();

    do {
        vector<string> ss;
        split(m_addr, "?", &ss);
        string addr = string("shm_tq_ipc_") + ss[0].substr(6);
        m_svr_shmem = new myutils::FileMapping();
        if (!m_svr_shmem) break;
        if (!m_svr_shmem->open_shmem(addr, sizeof(ConnectionSlotInfo), false))
            break;

        m_slot_info = (ConnectionSlotInfo*)m_svr_shmem->addr();
        if (m_slot_info->slot_size != sizeof(ConnectionSlot)) break;

        if (m_socket != INVALID_SOCKET)
            closesocket(m_socket);

        m_socket = connect_socket("127.0.0.1", m_slot_info->svr_port);
        if (m_socket == INVALID_SOCKET || !myutils::check_connect(m_socket, 2))
            break;

        set_socket_nonblock(m_socket, false);
        m_my_id = (uint64_t)myutils::random();
        int r = ::send(m_socket, (const char*)&m_my_id, sizeof(m_my_id), 0);
        if (r != sizeof(m_my_id))
            break;

        uint64_t slot_pos = 0;
        r = ::recv(m_socket, (char*)&slot_pos, sizeof(slot_pos), 0);
        if (r != sizeof(slot_pos))
            break;
	
        if (slot_pos >= m_slot_info->slot_count) break;

        auto slot = m_slot_info->slots + slot_pos;
        if (slot->client_id != m_my_id) break;

        m_slot = slot;
        set_socket_nonblock(m_socket, true);

        m_my_shmem = new myutils::FileMapping();
        if (!m_my_shmem->open_shmem(m_slot->shmem_name, m_slot->shmem_size, false))
            break;

        m_sem_recv = SharedSemaphore::open(m_slot->sem_send);
        m_sem_send = SharedSemaphore::open(m_slot->sem_recv);
        if (!m_sem_recv || !m_sem_send) break;

        ShmemHead* head = (ShmemHead*)m_my_shmem->addr();

        // different between client and server
        m_send_queue = (ShmemQueue*)(m_my_shmem->addr() + head->recv_offset);
        m_send_queue->init(head->recv_size);
        m_recv_queue = (ShmemQueue*)(m_my_shmem->addr() + head->send_offset);
        m_recv_queue->init(head->send_size);

        set_conn_stat(true);
        m_should_exit = false;
        m_recv_thread = new thread(bind(&IpcConnection::recv_run, this));

        if (m_callback)
            m_callback->on_conn_status(true);

        return true;

    } while (false);

    clear_data();

    return false;
}

void IpcConnection::close()
{   
    close_loop();
    clear_data();
}

void IpcConnection::send(const string& data)
{
    send(data.c_str(), data.size());
}

void IpcConnection::send(const char* data, size_t size)
{
    unique_lock<mutex> lock(m_send_mtx);
    if (m_connected && m_send_queue) {
        if (m_send_queue->push(data, size)) {
            m_sem_send->post();
        }
        else {
            //cout << "send error: failed to push\n";
            msg_loop().post_task([this]() {
                if (m_connected) {
                    m_connected = false;
                    if (m_callback) m_callback->on_conn_status(false);
                    do_connect();
                }
            });
        }
    }
    else {
        //cout << "send error: no connection\n";
    }
}

bool IpcConnection::is_connected()
{
    return m_connected;
}


