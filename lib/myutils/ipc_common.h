#ifndef _MYUTILS_IPC_COMMON_H
#define _MYUTILS_IPC_COMMON_H

#ifndef _WIN32
# include <semaphore.h>
# include <pthread.h>
#else
# include <WinSock2.h>
# include <Windows.h>
#endif

#ifndef _WIN32
static_assert( sizeof(pthread_mutex_t) + sizeof(pthread_cond_t) < 128, "shared semaphore in ConnectionInfo");
#endif

namespace myutils {

    using namespace std;

    class SharedSemaphore {
        SharedSemaphore();
    public:
        ~SharedSemaphore();        

        static SharedSemaphore* create(const char* name_or_mem);
        static SharedSemaphore* open(const char* name_or_mem);

        int  timed_wait(int timeout_ms);
        bool post();

#ifdef _WIN32
        HANDLE m_hSemaphore;
#elif defined(__linux__)
        sem_t* m_sem;
        sem_t  m_unamed_sem;
#else
        struct PthreadData {
            pthread_cond_t  cond;
            pthread_mutex_t mtx;
            int32_t        count;
        };
        PthreadData* m_data;
        PthreadData  m_data_not_shared;
#endif
    };

}

#endif

