#ifndef _MYUTILS_IPC_CONNECTION_H
#define _MYUTILS_IPC_CONNECTION_H

#include <atomic>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include "myutils/connection.h"
#include "myutils/filemapping.h"
#include "myutils/shm_queue.h"
#include "myutils/socketutils.h"
#include "myutils/ipc_common.h"
#include "myutils/loop/MsgRunLoop.h"

#ifndef _WIN32
static_assert( sizeof(pthread_mutex_t) + sizeof(pthread_cond_t) < 128, "shared semaphore in ConnectionInfo");
#endif

namespace myutils {

    using namespace std;
    using namespace std::chrono;

    class IpcConnection : public Connection, public loop::MsgLoopRun {
    public:
        struct ShmemHead {
            int32_t send_size;
            int32_t send_offset;
            int32_t recv_size;
            int32_t recv_offset;
        };

        struct ConnectionSlot {
            atomic<uint64_t> client_id;          // 0 means no connection;
            atomic<int32_t > dead_flag;
            int32_t shmem_size;
            char    shmem_name[128];
            char    sem_send[128];
            char    sem_recv[128];
        };

        struct ConnectionSlotInfo {
            uint32_t       svr_port;
            uint32_t       slot_count;
            uint32_t       slot_size;          // Size of ConnectionInfo
            ConnectionSlot slots[100];
        };

        IpcConnection();

        virtual ~IpcConnection();

        virtual bool connect(const std::string& addr, Connection_Callback* callback) override;
        virtual void reconnect() override;
        virtual void close() override;
        virtual void send(const char* data, size_t size) override;
        virtual void send(const std::string& data) override;
        virtual bool is_connected() override;

    private:
        bool do_connect();
        void do_recv();
        void recv_run();
        void clear_data();
        void check_connection();
        void on_idle_timer();

        inline void set_conn_stat(bool connected) {
            unique_lock<mutex> lock(m_send_mtx);
            m_connected = connected;
        }

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
        ConnectionSlot*             m_slot;
        SharedSemaphore*            m_sem_send;
        SharedSemaphore*            m_sem_recv;
        system_clock::time_point    m_last_connect_time;
        SOCKET                      m_socket;
    };
}

#endif

