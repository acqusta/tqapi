#include <assert.h>
#include <algorithm>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <string.h>
#include "myutils/misc.h"
#include "myutils/shmem_connection.h"
#include "myutils/stringutils.h"

bool Pipe::connect(const string& name)
{
    m_hPipe = CreateFileA(
        name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    return (m_hPipe != INVALID_HANDLE_VALUE);
}


int32_t  Pipe::recv(char* buf, int32_t size)
{
    if (m_hPipe != INVALID_HANDLE_VALUE) {
        DWORD read_len = 0;
        if (ReadFile(m_hPipe, buf, size, &read_len, 0))
            return read_len;
    }
    return -1;
}


int32_t Pipe::send(const char* data, int32_t size)
{
    if (m_hPipe != INVALID_HANDLE_VALUE) {
        DWORD read_len = 0;
        if (WriteFile(m_hPipe, data, size, &read_len, 0))
            return read_len;
    }
    return -1;
}


bool ShmemQueue::push(const char* data, size_t size)
{
    bool ret = false;
    auto begin_time = system_clock::now();
    do {
        atomic<int32_t> lock(m_write_mtx);
        int32_t v = 0;
        if (lock.compare_exchange_strong(v, 1)) {
            if (m_read_pos < m_write_pos) {
                // ---R----W---
                if (m_data_size - m_write_pos >= size + 4) {
                    // after ---R-------W-
                    int32_t len = (int32_t)size;
                    char* p = m_data + m_write_pos;
                    memcpy(p, (char*)&len, 4);
                    memcpy(p + 4, data, size);
                    m_write_pos += (int32_t)size + 4;
                    ret = true;
                }
                else if (m_read_pos > size + 4) {
                    // after  ---W---R-------
                    m_data_size = m_write_pos;
                    int32_t len = (int32_t)size;
                    char* p = m_data;
                    memcpy(p, (char*)&len, 4);
                    memcpy(p + 4, data, size);
                    m_write_pos = (int32_t)size + 4;
                    ret = true;
                }
                else {
                    assert(false);
                }
            }
            else if (m_read_pos - m_write_pos > size + 4) {
                //       ---W-------R----
                // after -------W---R----
                int32_t len = (int32_t)size;
                char* p = m_data + m_write_pos;
                memcpy(p, (char*)&len, 4);
                memcpy(p + 4, data, size);
                m_write_pos = (int32_t)size + 4;
                ret = true;
                return false;
            }
            lock--;
            return true;
        }
    } while (system_clock::now() - begin_time < milliseconds(10));

    return false;
}

bool ShmemQueue::poll(const char** data, size_t* size)
{
    bool ret = false;
    auto begin_time = system_clock::now();
    do {
        atomic<int32_t> lock(m_read_mtx);
        int32_t v = 0;
        if (lock.compare_exchange_strong(v, 1)) {
            if (m_read_pos < m_write_pos) {
                char* p = m_data + m_read_pos;
                int32_t pkt_size = *(int32_t*)p;
                *data = p + 4;
                *size = pkt_size;
                assert(pkt_size <= m_write_pos - m_read_pos);
                ret = true;
            }
            else if (m_read_pos > m_write_pos) {
                char* p = m_data + m_read_pos;
                int32_t pkt_size = *(int32_t*)p;
                *data = p + 4;
                *size = pkt_size;
                assert(*size == m_data_realsize - m_read_pos);
                ret = true;
            }
            lock--;
            return ret;
        }

    } while (system_clock::now() - begin_time < milliseconds(10));

    return false;
}

bool ShmemQueue::pop()
{
    bool ret = false;
    auto begin_time = system_clock::now();
    do {
        atomic<int32_t> lock(m_read_mtx);
        int32_t v = 0;
        if (lock.compare_exchange_strong(v, 1)) {
            char* p = m_data + m_read_pos;
            int32_t pkt_size = *(int32_t*)p;

            if (m_read_pos < m_write_pos) {
                m_read_pos += pkt_size + 4;
                assert(m_read_pos <= m_write_pos);
                if (m_read_pos == m_write_pos) {
                    m_read_pos = m_write_pos = 0;
                    m_data_size = m_data_realsize; // Just ensure
                }
            }
            else if (m_read_pos > m_write_pos) {
                m_read_pos += pkt_size + 4;
                assert(m_read_pos <= m_data_size);
                if (m_read_pos == m_data_size) {
                    m_read_pos = 0;
                    m_data_size = m_data_realsize;
                }
            }

            lock--;
            return ret;
        }
    } while (system_clock::now() - begin_time < milliseconds(10));

    return false;
}


ShmemConnection::ShmemConnection()
    : m_callback(nullptr)
    , m_should_exit(false)
    , m_connected(false)
    , m_shmem(nullptr)
    , m_recv_queue(nullptr)
    , m_send_queue(nullptr)
{
}

ShmemConnection::~ShmemConnection()
{
    close();
}

void ShmemConnection::do_close()
{
    m_connected = false;

    if (m_callback) m_callback->on_conn_status(false);
}

void ShmemConnection::recv_run()
{
#ifndef _WIN32
    char buf[1024];

    while (!m_should_exit) {
        if (!m_pipe) break;
        int r = m_pipe->recv(buf, 4, 1000);
        if (r == 0) continue;
        if (r != 4) break;
        uint32_t pkt_size = *(uint32_t*)buf;
        if (pkt_size > 1024) break;
        r = m_pipe->recv(buf, pkt_size, 1000);
        if (r != pkt_size) break;

        ServerMsg* msg = (ServerMsg*)buf;
        if (msg->msg_id == MSGID_DATA_ARRIVED)
            msg_loop().PostTask(bind(&ShmemConnection::do_recv, this)); break;
    }
#else
    while (!m_should_exit) {
        if (WaitForSingleObject(m_hRecvEvt, 1000) == WAIT_OBJECT_0) {
            ResetEvent(m_hRecvEvt);
            msg_loop().PostTask(bind(&ShmemConnection::do_recv, this)); 
            break;
        }
    }
#endif
    
    msg_loop().PostTask([this]() {
        if (m_connected) {
            m_connected = false;
            if (m_callback) m_callback->on_conn_status(false);
            do_connect();
        }
    });

}


void ShmemConnection::do_recv()
{
    const char* data;
    size_t size;

    // at most read 10 msgs in one loop
    for (int i = 0; i < 10; i++) {
        if (m_recv_queue->poll(&data, &size)) {
            if (m_callback) m_callback->on_recv(data, size);
            m_recv_queue->pop();
        }
        else {
            break;
        }
    }
}

bool ShmemConnection::connect(const string& addr, Connection_Callback* callback)
{
    mutex mtx;
    condition_variable cv;

    unique_lock<mutex> lock(mtx);

    m_msg_loop.PostTask([this, &cv, addr, callback]() {
        m_addr = addr;
        m_callback = callback;
        do_connect();
        cv.notify_all();
    });

    cv.wait(lock);

    return m_connected;
}

void ShmemConnection::reconnect()
{
    //msg_loop().PostTask(bind(&ShmemConnection::do_connect, this));
}

bool ShmemConnection::do_connect() 
{
    if (m_connected) return true;

    if (m_addr.empty()) return false;

    if (m_recv_thread) {
        m_should_exit = true;
    }

    if (m_pipe) {
        delete m_pipe;
        m_pipe = nullptr;
    }
    if (m_shmem) {
        delete m_shmem;
        m_shmem = nullptr;
    }

    string addr; addr.resize(m_addr.size());
    char* p = (char*)addr.c_str();
    for (size_t i = 0; i < m_addr.size(); i++) {
        switch (m_addr[i]) {
        case '\\':
        case '/':
        case ':':
        case ' ':
            *p++ = '_';
            break;
        default:
            *p++ = tolower(m_addr[i]);
        }
    }
#ifdef _WIN32
    addr = string("\\\\.\\pipe\\") + addr;
#endif
    Pipe* pipe = new Pipe;
    char buf[1024];
    do {
        if (!pipe->connect(addr)) break;

        m_shmem = new myutils::FileMapping();
        int id = myutils::random();
        sprintf(buf, "shm_%d", id);
        if (!m_shmem->create_shmem(buf, 22 * 1024 * 1024)) {
            delete m_shmem;
            m_shmem = nullptr;
            break;
        }
        ShmemHead* head = (ShmemHead*)m_shmem->addr();
        head->send_size = 2 * 1024 * 1024 - sizeof(ShmemHead);
        head->send_offset = sizeof(ShmemHead);
        head->recv_size = 20 * 1024 * 1024;
        head->recv_offset = 2 * 1024 * 1024;

        m_recv_queue = (ShmemQueue*)(m_shmem->addr() + head->recv_offset);
        m_recv_queue->init(head->recv_size);
        m_send_queue = (ShmemQueue*)(m_shmem->addr() + head->send_offset);
        m_send_queue->init(head->send_size);

        ConnectReq req;
        memset(&req, 0, sizeof(req));
        req.msg_id = MSGID_CONNECT_REQ;
        strcpy(req.shmem_name, m_shmem->id().c_str());
#ifdef _WIN32
        sprintf(req.evt_send, "shm_send_%d", id);
        sprintf(req.evt_recv, "shm_recv_%d", id);
#endif

        if (!pipe->send((const char*)&req, sizeof(req))) {
            delete pipe;
            return false;
        }

        if (pipe->recv(buf, 4) != 4) break;
        int32_t pkt_size = *(int32_t*)buf;
        if (pkt_size < sizeof(ConnectRsp) || pkt_size > 1024) break;        
        if (pkt_size == pipe->recv(buf, pkt_size)) break;

        ConnectRsp* rsp = (ConnectRsp*)buf;
        if (!rsp->conn_id) break;
        m_conn_id = rsp->conn_id;
        m_connected = true;
        if (m_callback) m_callback->on_conn_status(true);

#ifndef _WIN32
        m_pipe = pipe;
#else
        m_hSendEvt = CreateEventA(NULL, TRUE, FALSE, req.evt_send);
        m_hRecvEvt = CreateEventA(NULL, TRUE, FALSE, req.evt_recv);

#endif
        m_recv_thread = new thread(bind(&ShmemConnection::recv_run, this));
        return true;
    } while (false);

    if (pipe) delete pipe;
    return false;
}

void ShmemConnection::close()
{   
    close_loop();
    m_should_exit = true;
    if (m_recv_thread) {
        m_recv_thread->join();
        m_recv_thread = nullptr;
    }
#ifndef _WIN32
    if (m_pipe) {
        delete m_pipe;
        m_pipe = nullptr;
    }
#endif

    m_recv_queue = nullptr;
    m_send_queue = nullptr;

    if (m_shmem) {
        delete m_shmem;
        m_shmem = nullptr;
    }
}


void ShmemConnection::send(const string& data)
{
    send(data.c_str(), data.size());
}

void ShmemConnection::send(const char* data, size_t size)
{
    unique_lock<mutex> lock(m_send_mtx);
    if (m_connected && m_send_queue) {
        if (!m_send_queue->push(data, size)) {
            msg_loop().PostTask([this]() {
                if (m_connected) {
                    m_connected = false;
                    if (m_callback) m_callback->on_conn_status(false);
                    do_connect();
                }
            });
        }
    }
}