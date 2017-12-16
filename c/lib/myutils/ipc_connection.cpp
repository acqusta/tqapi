#include <assert.h>
#include <algorithm>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <string.h>
#include "myutils/misc.h"
#include "myutils/ipc_connection.h"
#include "myutils/stringutils.h"

using namespace myutils;

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


IpcConnection::IpcConnection()
    : m_callback(nullptr)
    , m_should_exit(false)
    , m_connected(false)
    , m_conn_id(0)
    , m_shmem(nullptr)
    , m_recv_queue(nullptr)
    , m_send_queue(nullptr)
    , m_pipe(nullptr)
    , m_recv_thread(nullptr)
{
}

IpcConnection::~IpcConnection()
{
    close();
}

void IpcConnection::do_close()
{
    m_connected = false;

    if (m_callback) m_callback->on_conn_status(false);
}

void IpcConnection::recv_run()
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
            msg_loop().PostTask(bind(&IpcConnection::do_recv, this)); break;
    }
#else
    while (!m_should_exit) {
        switch(WaitForSingleObject(m_hRecvEvt, 1000)) {
        case WAIT_OBJECT_0:
            ResetEvent(m_hRecvEvt);
            msg_loop().PostTask(bind(&IpcConnection::do_recv, this)); 
            break;
        case WAIT_TIMEOUT:
            continue;
            break;
        default:
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


void IpcConnection::do_recv()
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

    if (m_recv_queue->poll(&data, &size)) {
        m_msg_loop.PostTask(bind(&IpcConnection::do_recv, this));
    }
}

bool IpcConnection::connect(const string& addr, Connection_Callback* callback)
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

void IpcConnection::reconnect()
{
    //msg_loop().PostTask(bind(&IpcConnection::do_connect, this));
}

bool IpcConnection::do_connect() 
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
    
    string addr = m_addr.substr(6);
#ifdef _WIN32
    addr = string("\\\\.\\pipe\\") + addr;
#endif
    Pipe* pipe = new Pipe;
    char buf[1024];
    do {
        m_shmem = new myutils::FileMapping();
        int id = myutils::random();
        sprintf(buf, "shm%d", id);
        if (!m_shmem->create_shmem(buf, 30 * 1024 * 1024)) {
            delete m_shmem;
            m_shmem = nullptr;
            break;
        }

        ConnectReq req;
        memset(&req, 0, sizeof(req));
        req.msg_size = sizeof(req);
        req.msg_id = MSGID_CONNECT_REQ;
        strcpy(req.shmem_name, m_shmem->id().c_str());
#ifdef _WIN32
        sprintf(req.evt_send, "Global\\ipc_evt_send_%d", id);
        sprintf(req.evt_recv, "Global\\ipc_evt_recv_%d", id);
#endif

        ShmemHead* head = (ShmemHead*)m_shmem->addr();
        head->recv_size   = 2 * 1024 * 1024 - sizeof(ShmemHead);
        head->recv_offset = sizeof(ShmemHead);
        head->send_size   = 28 * 1024 * 1024;
        head->send_offset = 2 * 1024 * 1024;

        // different between client and server

        m_send_queue = (ShmemQueue*)(m_shmem->addr() + head->recv_offset);
        m_send_queue->init(head->recv_size);
        m_recv_queue = (ShmemQueue*)(m_shmem->addr() + head->send_offset);
        m_recv_queue->init(head->send_size);

        SECURITY_DESCRIPTOR secu_desc;
        ::InitializeSecurityDescriptor(&secu_desc, SECURITY_DESCRIPTOR_REVISION);
        ::SetSecurityDescriptorDacl(&secu_desc, TRUE, NULL, FALSE);
        SECURITY_ATTRIBUTES secu_attr;
        secu_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
        secu_attr.bInheritHandle = FALSE;
        secu_attr.lpSecurityDescriptor = &secu_desc;

        // different between client and server
        m_hRecvEvt = CreateEventA(&secu_attr, TRUE, FALSE, req.evt_send);
        m_hSendEvt = CreateEventA(&secu_attr, TRUE, FALSE, req.evt_recv);

        if (!pipe->connect(addr)) break;
        if (!pipe->send((const char*)&req, sizeof(req))) {
            delete pipe;
            return false;
        }

        if (pipe->recv(buf, sizeof(ConnectRsp)) != sizeof(ConnectRsp)) break;
        ConnectRsp* rsp = (ConnectRsp*)buf;
        if (rsp->msg_id != MSGID_CONNECT_RSP) break;

        m_conn_id = rsp->conn_id;
        m_connected = true;
        if (m_callback) m_callback->on_conn_status(true);

        m_pipe = pipe;
        m_recv_thread = new thread(bind(&IpcConnection::recv_run, this));

        return true;
    } while (false);

    if (pipe) delete pipe;
    return false;
}

void IpcConnection::close()
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


void IpcConnection::send(const string& data)
{
    send(data.c_str(), data.size());
}

void IpcConnection::send(const char* data, size_t size)
{
    unique_lock<mutex> lock(m_send_mtx);
    if (m_connected && m_send_queue) {
        if (m_send_queue->push(data, size)) {
            SetEvent(m_hSendEvt);
        }
        else {
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