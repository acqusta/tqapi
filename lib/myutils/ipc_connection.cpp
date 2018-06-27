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

SharedSemaphore::SharedSemaphore()
#ifdef _WIN32
    : m_hSemaphore(nullptr)
#elif defined(__linux__)
    : m_sem(nullptr)
#else
    : m_data(nullptr)
#endif
{
}

SharedSemaphore::~SharedSemaphore()
{
#ifdef _WIN32
    if (m_hSemaphore) CloseHandle(m_hSemaphore);
#elif defined(__linux__)
    if (m_sem && m_sem != SEM_FAILED)
        sem_close(m_sem);
#endif
}

SharedSemaphore*  SharedSemaphore::create(const char* name)
{
#ifdef _WIN32
    HANDLE h = CreateSemaphoreA(NULL, 0, 1000000, name);
    if (h != nullptr) {
        auto sem = new SharedSemaphore();
        sem->m_hSemaphore = h;
        return sem;
    } else {
        return nullptr;
    }
#elif defined(__linux__)
    auto sem = new SharedSemaphore();
    sem->m_sem = sem_open(name, O_CREAT, 0666, 0);
    if (sem->m_sem != SEM_FAILED) {
        return sem;
    } else {
        delete sem;
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
    HANDLE h = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, name);
    if (h != nullptr) {
        auto sem = new SharedSemaphore();
        sem->m_hSemaphore = h;
        return sem;
    } else {
        return nullptr;
    }
#elif defined(__linux__)
    auto sem = new SharedSemaphore();
    sem->m_sem = sem_open(name, 0);
    if (sem->m_sem != SEM_FAILED) {
        return sem;
    } else {
        delete sem;
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
    switch(WaitForSingleObject(m_hSemaphore, timeout_ms)){
    case WAIT_OBJECT_0:
        return 1;
    case WAIT_TIMEOUT:
        return 0;
    default:
        return -1;
    }
    
#elif defined(__linux__)

    struct timeval now;
    struct timespec outtime;

    gettimeofday(&now, NULL);
    memset(&outtime, 0, sizeof(outtime));
    outtime.tv_sec  = now.tv_sec + timeout_ms/1000;
    outtime.tv_nsec = now.tv_usec * 1000 + (timeout_ms%1000) * 1000000;
    outtime.tv_sec  += outtime.tv_nsec / 1000000000;
    outtime.tv_nsec %= 1000000000;
    int r = sem_timedwait(m_sem, &outtime);
    //cout << "wait " << r << "," << m_data->count << "," << timeout_ms << endl;
    switch(r) {
    case 0:            return 1;
    case ETIMEDOUT:    return 0;
    case EAGAIN:       return 0;
    default:           return -1;
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
        int ret = 0;
        //cout << "wait " << r << "," << m_data->count << "," << timeout_ms << endl;
        if (m_data->count > 0) {
            m_data->count--;
            ret = 1;
        }
        pthread_mutex_unlock (&m_data->mtx);

        switch(r) {
        case 0:            return ret;
        case ETIMEDOUT:    return 0;
        default:           return -1;
        }

    } else {
        //cout << "wait has value " << m_data->count << endl;
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
    if (m_hSemaphore) {
        ReleaseSemaphore(m_hSemaphore, 1, NULL);
        return true;
    } else {
        return false;
    }
#elif defined(__linux__)
    return sem_post(m_sem) == 0;
#else
    if (m_data) {
        pthread_mutex_lock(&m_data->mtx);
        m_data->count++;
        //cout << "post " << m_data->count << endl;
        pthread_cond_signal(&m_data->cond);
        pthread_mutex_unlock(&m_data->mtx);
        return true;
    } else {
        return false;
    }
#endif
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
    , m_slot(nullptr)
    , m_sem_send(nullptr)
    , m_sem_recv(nullptr)
    , m_socket(INVALID_SOCKET)
{
    m_msg_loop.PostDelayedTask(bind(&IpcConnection::check_connection, this),  500);
    m_msg_loop.PostDelayedTask(bind(&IpcConnection::on_idle_timer,    this), 1000);
}

IpcConnection::~IpcConnection()
{
    close();
}

void IpcConnection::recv_run()
{
    while (!m_should_exit) {
        if (m_slot->client_id != m_my_id)  break;
#if 1
        switch (m_sem_recv->timed_wait(1)) {
        case 1:
            msg_loop().PostTask([this]() {
                do_recv(); 
            });
            break;
        case 0:
            break;
        default:
            break;
        }
#else
        do_recv();
#endif
    }
    
    msg_loop().PostTask([this]() {
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

    //if (m_recv_queue->poll(&data, &size))
    //    m_msg_loop.PostTask(bind(&IpcConnection::do_recv, this));
}

bool IpcConnection::connect(const string& addr, Connection_Callback* callback)
{
    mutex mtx;
    condition_variable cv;
    unique_lock<mutex> lock(mtx);

    m_msg_loop.PostTask([this, &mtx, &cv, addr, callback]() {
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
    m_msg_loop.PostTask([this] {
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

    m_msg_loop.PostDelayedTask(bind(&IpcConnection::check_connection, this), 500);
}

void IpcConnection::on_idle_timer()
{
    m_msg_loop.PostDelayedTask(bind(&IpcConnection::on_idle_timer, this), 1000);
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
            msg_loop().PostTask([this]() {
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


