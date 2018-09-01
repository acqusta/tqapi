#include <string>
#include <memory>
#include <string.h>
#include <time.h>
#ifndef _WIN32
# include <sys/time.h>
# include <fcntl.h>
#endif
#include <errno.h>
#include "ipc_common.h"

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
    if (m_sem && m_sem != SEM_FAILED) {
        if (m_sem == &m_unamed_sem)
            sem_destroy(m_sem);
        else
            sem_close(m_sem);
    }
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
    if (name) {
        sem->m_sem = sem_open(name, O_CREAT, 0666, 0);
    }
    else {
        sem_init(&sem->m_unamed_sem, 0, 0);
        sem->m_sem = &sem->m_unamed_sem;
    }
    if (sem->m_sem != SEM_FAILED) {
        return sem;
    } else {
        delete sem;
        return nullptr;
    }
#else
    auto sem = new SharedSemaphore();

    sem->m_data = name ? (PthreadData*)name : &sem->m_data_not_shared;
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

    //cout << "wait " << r << "," << errno  << "," << timeout_ms << endl;
    if (r == -1) {
        switch(errno) {
        case ETIMEDOUT:    return 0;
        case EAGAIN:       return 0;
        case EINTR:       return 0;
        default:           return -1;
        }
    } else {
        return 1;
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


