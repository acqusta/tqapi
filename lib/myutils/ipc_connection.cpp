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

using namespace myutils;

static inline uint64_t get_now_ms()
{
    return time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count();
}

static inline int64_t get_now_micro()
{
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}


SharedSemaphore::SharedSemaphore()
#ifdef _WIN32
    : m_hEvent(nullptr)
#else
    : m_data(nullptr)
#endif
{
}

SharedSemaphore::~SharedSemaphore()
{
#ifdef _WIN32
    if (m_hEvent) CloseHandle(m_hEvent);
#endif
}

SharedSemaphore*  SharedSemaphore::create(const char* name)
{
#ifdef _WIN32
    HANDLE hEvent  = CreateEventA(NULL, TRUE, FALSE, name);
    if (hEvent != nullptr) {
        auto sem = new SharedSemaphore();
        sem->m_hEvent = hEvent;
        return sem;
    } else {
        return nullptr;
    }
#else
    auto sem = new SharedSemaphore();

    sem->m_data = (PthreadData*)name;
    sem->m_data->count = 0;

    pthread_condattr_t cond_shared_attr;  
    pthread_condattr_init (&cond_shared_attr);  
    pthread_condattr_setpshared (&cond_shared_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init (&sem->m_data->cond, &cond_shared_attr);  

    pthread_mutexattr_t mutex_shared_attr;  
    pthread_mutexattr_init (&mutex_shared_attr);  
    pthread_mutexattr_setpshared (&mutex_shared_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init (&sem->m_data->mtx, &mutex_shared_attr);

    return sem;
#endif
}

SharedSemaphore* SharedSemaphore::open(const char* name)
{
#ifdef _WIN32
    
    auto hEvent = OpenEventA(EVENT_ALL_ACCESS, FALSE, name);
    if (hEvent != nullptr) {
        auto sem = new SharedSemaphore();
        sem->m_hEvent = hEvent;
        return sem;
    } else {
        return nullptr;
    }
#else
    auto sem = new SharedSemaphore();
    sem->m_data = (PthreadData*)name;
    return sem;
#endif
}

//  1 -- got
//  0 -- timeout
// -1 -- error
int SharedSemaphore::timed_wait(int timeout_ms)
{
#ifdef _WIN32
    switch(WaitForSingleObject(m_hEvent, timeout_ms)){
    case WAIT_OBJECT_0:
        ResetEvent(m_hEvent);
        return 1;
    case WAIT_TIMEOUT:
        return 0;
    default:
        return -1;
    }
#else
    struct timeval now;
    struct timespec outtime;


    pthread_mutex_lock(&m_data->mtx);

    if (m_data->count <= 0 ) {
        gettimeofday(&now, NULL);
        memset(&outtime, 0, sizeof(outtime));
        outtime.tv_sec  = now.tv_sec + timeout_ms/1000;
        outtime.tv_nsec = now.tv_usec * 1000 + (timeout_ms%1000) * 1000000;
        outtime.tv_sec  += outtime.tv_nsec / 1000000000;
        outtime.tv_nsec %= 1000000000;
        int r = pthread_cond_timedwait(&m_data->cond, &m_data->mtx, &outtime);
        if (m_data->count>0)
            m_data->count--;
        pthread_mutex_unlock (&m_data->mtx);
        switch(r) {
        case 0:            return 1;
        case ETIMEDOUT:    return 0;
        default:           return -1;
        }

    } else {
        m_data->count--;
        pthread_mutex_unlock (&m_data->mtx);
        return 1;
    }
#endif
}

//  1 -- got
//  0 -- timeout
// -1 -- error
bool SharedSemaphore::post()
{
#ifdef _WIN32
    if (m_hEvent) {
        SetEvent(m_hEvent);
        return true;
    } else {
        return false;
    }
#else
    if (m_data) {
        pthread_mutex_lock(&m_data->mtx);
        m_data->count++;
        pthread_cond_signal(&m_data->cond);
        pthread_mutex_unlock(&m_data->mtx);
        return true;
    } else {
        return false;
    }
#endif
}

IpcConnection::IpcConnection(int32_t shmem_size)
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
    , m_sem_send(nullptr)
    , m_sem_recv(nullptr)
    , m_shmem_size(shmem_size)
{
    m_msg_loop.PostDelayedTask(bind(&IpcConnection::check_connection, this), 500);
}

IpcConnection::~IpcConnection()
{
    close();
}

void IpcConnection::recv_run()
{
    auto last_idle_time = system_clock::now();
    while (!m_should_exit) {
        if (m_conn->client_id != m_my_id)  break;
        if (get_now_ms() - m_conn->svr_update_time > 2000) break;

        switch (m_sem_recv->timed_wait(100)) {
        case 1:
            msg_loop().PostTask([this]() { do_recv(); });
            break;
        case 0:
            m_conn->client_update_time = get_now_ms();
            break;
        default:
            break;
        }

        if (system_clock::now() - last_idle_time > seconds(1)) {
            msg_loop().PostTask([this]() { if (m_callback) m_callback->on_idle(); });
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
        if (m_recv_queue->poll(&data, &size) ) {
            if (m_callback)
                m_callback->on_recv(data, size);
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

    if (m_sem_recv) {
        delete m_sem_recv;
        m_sem_recv = nullptr;
    }

    if (m_sem_send) {
        delete m_sem_send;
        m_sem_send = nullptr;
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
            sprintf(buf, "shm_%u", (uint32_t)m_my_id);

            m_my_shmem = new myutils::FileMapping();
            if (!m_my_shmem->create_shmem(buf, m_shmem_size))
                break;
            strcpy(m_conn->shmem_name, m_my_shmem->id().c_str());
            m_conn->shmem_size = m_shmem_size;
        }

#ifdef _WIN32        
        sprintf(m_conn->sem_send, "ipc_sem_send_%ud", (uint32_t)m_my_id);
        sprintf(m_conn->sem_recv, "ipc_sem_recv_%ud", (uint32_t)m_my_id);
#endif
        m_sem_recv = SharedSemaphore::create(m_conn->sem_send);
        m_sem_send = SharedSemaphore::create(m_conn->sem_recv);

        ShmemHead* head = (ShmemHead*)m_my_shmem->addr();
        head->recv_size   = 2 * 1024 * 1024 - sizeof(ShmemHead);
        head->recv_offset = sizeof(ShmemHead);
        head->send_size   = m_shmem_size - 2 * 1024 * 1024;
        head->send_offset = 2 * 1024 * 1024;

        // different between client and server
        m_send_queue = (ShmemQueue*)(m_my_shmem->addr() + head->recv_offset);
        m_send_queue->init(head->recv_size);
        m_recv_queue = (ShmemQueue*)(m_my_shmem->addr() + head->send_offset);
        m_recv_queue->init(head->send_size);

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
        }

        auto sem = SharedSemaphore::open(m_slot_info->sem_conn);
        if (sem) {
            sem->post();
            delete sem;
        } else {
            break;
        }

        {
            auto begin_time = system_clock::now();
            while (duration_cast<milliseconds>(system_clock::now() - begin_time).count() < 200) {
                if (m_conn->rsp) break;
            }
            if (m_conn->rsp != 1)
                break;
        }

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
            m_sem_send->post();
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

