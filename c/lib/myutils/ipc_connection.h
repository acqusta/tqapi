#ifndef _MYUTILS_IPC_CONNECTION_H
#define _MYUTILS_IPC_CONNECTION_H

#include <atomic>
#include <list>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#ifndef _WIN32
# include <pthread.h>
#endif
#include "myutils/connection.h"
#include "myutils/filemapping.h"
#include "myutils/shm_queue.h"
#include "myutils/socketutils.h"
#include "loop/MsgRunLoop.h"

#ifndef _WIN32
static_assert( sizeof(pthread_mutex_t) + sizeof(pthread_cond_t) < 128, "shared semaphore in ConnectionInfo");
#endif

namespace myutils {

    using namespace std;
    using namespace std::chrono;

    class SharedSemaphore {
        SharedSemaphore();
    public:
        ~SharedSemaphore();        

        static SharedSemaphore* create(const char* name_or_mem);
        static SharedSemaphore* open(const char* name_or_mem);

        int  timed_wait(int timeout_ms);
        bool post();

#ifdef _WIN32
        HANDLE m_hEvent;
#else
        pthread_cond_t*  m_cond;
        pthread_mutex_t* m_mtx;
#endif
    };

    class IpcConnection : public Connection, public loop::MsgLoopRun {
    public:
        struct ShmemHead {
            int32_t send_size;
            int32_t send_offset;
            int32_t recv_size;
            int32_t recv_offset;
        };

        struct ConnectionInfo {
            atomic<uint64_t> client_id;          // 0 means no connection;
            atomic<uint64_t> client_update_time; // client update time
            atomic<uint64_t> svr_update_time;    // server check time
            atomic<int32_t > req;
            atomic<int32_t > rsp;
            atomic<int32_t > dead_flag;
            char shmem_name[128];
            char sem_send[128];
            char sem_recv[128];
        };

        struct ConnectionSlotInfo {
            int64_t        slot_count;
            int64_t        slot_size;          // Size of ConnectionInfo
            char           sem_conn[128];
            ConnectionInfo slots[20];
        };

        IpcConnection();

        virtual ~IpcConnection();

        virtual bool connect(const std::string& addr, Connection_Callback* callback) override;
        virtual void reconnect() override;
        virtual void close() override;
        virtual void send(const char* data, size_t size) override;
        virtual void send(const std::string& data) override;

    private:
        bool do_connect();
        void do_recv();
        void do_close();
        void recv_run();
        void clear_data();
        void check_connection();

    private:
        string                      m_addr;
        mutex                       m_send_mtx;
        Connection_Callback*        m_callback;
        volatile bool               m_should_exit;
        bool                        m_connected;
        uint64_t                    m_my_id;
        myutils::FileMapping*       m_my_shmem;
        myutils::FileMapping*       m_svr_shmem;
        ShmemQueue*                 m_recv_queue;
        ShmemQueue*                 m_send_queue;
        thread*                     m_recv_thread;
        ConnectionSlotInfo*         m_slot_info;
        ConnectionInfo*             m_conn;
        SharedSemaphore*            m_sem_send;
        SharedSemaphore*            m_sem_recv;
    };
}

#endif

