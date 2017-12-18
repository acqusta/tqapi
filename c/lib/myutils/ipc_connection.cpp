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

static inline uint64_t get_now_ms()
{
    return time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count();
}

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
    , m_conn(nullptr)
{
    m_msg_loop.PostDelayedTask(bind(&IpcConnection::check_connection, this), 500);
}

IpcConnection::~IpcConnection()
{
    close();
}

void IpcConnection::recv_run()
{
    while (!m_should_exit) {
        if (m_conn->client_id != m_my_id)  break;
        if (get_now_ms() - m_conn->svr_update_time > 2000) break;

        switch(WaitForSingleObject(m_hRecvEvt, 100)) {
        case WAIT_OBJECT_0:
            ResetEvent(m_hRecvEvt);
            msg_loop().PostTask(bind(&IpcConnection::do_recv, this)); 
            break;
        case WAIT_TIMEOUT:
            m_conn->client_update_time = get_now_ms();
            break;
        default:
            break;
        }
    }
    
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

    if (m_recv_queue->poll(&data, &size))
        m_msg_loop.PostTask(bind(&IpcConnection::do_recv, this));
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
}


void IpcConnection::clear_data()
{
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
    m_conn = nullptr;

    if (m_hRecvEvt != nullptr) {
        CloseHandle(m_hRecvEvt);
        m_hRecvEvt = nullptr;
    }

    if (m_hSendEvt != nullptr) {
        CloseHandle(m_hSendEvt);
        m_hSendEvt = nullptr;
    }
}

void IpcConnection::check_connection()
{
    if (!m_connected) do_connect();

    m_msg_loop.PostDelayedTask(bind(&IpcConnection::check_connection, this), 500);
}

bool IpcConnection::do_connect() 
{
    if (m_connected) return true;

    if (m_addr.empty()) return false;

    clear_data();

    //std::cout << "connect to " << m_addr << endl;

    do {
        string addr = string("shm_tqc_v1_") + m_addr.substr(6);
        m_svr_shmem = new myutils::FileMapping();
        if (!m_svr_shmem) break;
        if (!m_svr_shmem->open_shmem(addr, sizeof(ConnectionSlotInfo), false)) {
            //std::cerr << "can't open shmem " << addr << endl;
            break;
        }

        m_slot_info = (ConnectionSlotInfo*)m_svr_shmem->addr();
        if (m_slot_info->slot_size != sizeof(ConnectionInfo)) break;

        m_my_id = myutils::random();
        //std::cout << "Set my id: " << m_my_id << endl;

        for (int i = 0; i < m_slot_info->slot_count; i++) {
            auto slot = m_slot_info->slots + i;
            uint64_t exp = 0;
            if (slot->client_id.compare_exchange_strong(exp, m_my_id)) {
                //std::cout << "Find empty slot " << i << endl;
                m_conn = slot;
                break;
            }
        }
    
        if (!m_conn) {
            //std::cerr << "Can't find empty slot!" << endl;
            break;
        }

        {
            char buf[100];
            sprintf(buf, "shm_%ud", (uint32_t)m_my_id);

            m_my_shmem = new myutils::FileMapping();
            if (!m_my_shmem->create_shmem(buf, 30 * 1024 * 1024))
                break;
            strcpy(m_conn->shmem_name, m_my_shmem->id().c_str());
        }

#ifdef _WIN32
        sprintf(m_conn->evt_send, "ipc_evt_send_%ud", (uint32_t)m_my_id);
        sprintf(m_conn->evt_recv, "ipc_evt_recv_%ud", (uint32_t)m_my_id);
#endif

        ShmemHead* head = (ShmemHead*)m_my_shmem->addr();
        head->recv_size   = 2 * 1024 * 1024 - sizeof(ShmemHead);
        head->recv_offset = sizeof(ShmemHead);
        head->send_size   = 28 * 1024 * 1024;
        head->send_offset = 2 * 1024 * 1024;

        // different between client and server
        m_send_queue = (ShmemQueue*)(m_my_shmem->addr() + head->recv_offset);
        m_send_queue->init(head->recv_size);
        m_recv_queue = (ShmemQueue*)(m_my_shmem->addr() + head->send_offset);
        m_recv_queue->init(head->send_size);

#ifdef _WIN32
        // different between client and server
        m_hRecvEvt = CreateEventA(NULL, TRUE, FALSE, m_conn->evt_send);
        m_hSendEvt = CreateEventA(NULL, TRUE, FALSE, m_conn->evt_recv);
#endif
        {
            uint64_t exp = 0;
            if (!m_conn->client_update_time.compare_exchange_strong(exp, get_now_ms())) {
                //std::cerr << "update_time is not zero" << endl;
                break;
            }
        }

        {
            int32_t exp = 0;
            if (!m_conn->req.compare_exchange_strong(exp, 1)) break;

            HANDLE hConnEvt = OpenEventA(EVENT_ALL_ACCESS, FALSE, m_slot_info->evt_conn);
            if (hConnEvt == INVALID_HANDLE_VALUE) break;
            SetEvent(hConnEvt);
        }

        {
            auto begin_time = system_clock::now();
            while (duration_cast<milliseconds>(system_clock::now() - begin_time).count() < 200) {
                if (m_conn->rsp) break;
            }
            if (m_conn->rsp != 1) {
                //std::cerr << "wrong rsp " << m_conn->rsp << endl;
                break;
            }
        }

        //std::cout << "Connected\n";

        m_connected = true;

        m_should_exit = false;
        m_recv_thread = new thread(bind(&IpcConnection::recv_run, this));

        // connected
        if (m_callback) m_callback->on_conn_status(true);

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