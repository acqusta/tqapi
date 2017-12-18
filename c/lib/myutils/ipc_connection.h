#ifndef _MYUTILS_IPC_CONNECTION_H
#define _MYUTILS_IPC_CONNECTION_H

#ifdef _WIN32

#include <atomic>
#include <list>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include "myutils/connection.h"
#include "myutils/filemapping.h"
#include "myutils/shm_queue.h"
#include "myutils/socketutils.h"
#include "loop/MsgRunLoop.h"

namespace myutils {

    using namespace std;
    using namespace std::chrono;

//    class Pipe {
//    public:
//        bool connect(const string& name);
//        bool listen(const string& name);
//
//        int32_t  recv(char* buf, int32_t size);
//        int32_t  send(const char* data, int32_t size);
//
//    private:
//#ifdef _WIN32
//        HANDLE m_hPipe;
//#endif
//    };

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
#ifdef _WIN32
            char evt_send[128];
            char evt_recv[128];
#endif
        };

        struct ConnectionSlotInfo {
            int64_t slot_count;
            int64_t slot_size;          // Size of ConnectionInfo
            char evt_conn[128];
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
#ifdef _WIN32
        HANDLE m_hSendEvt;
        HANDLE m_hRecvEvt;
#endif
    };

}

#endif

#endif
