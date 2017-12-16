#ifndef _MYUTILS_IPC_CONNECTION_H
#define _MYUTILS_IPC_CONNECTION_H

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


    class Pipe {
    public:
        bool connect(const string& name);
        bool listen(const string& name);

        int32_t  recv(char* buf, int32_t size);
        int32_t  send(const char* data, int32_t size);

    private:
#ifdef _WIN32
        HANDLE m_hPipe;
#endif
    };

    class IpcConnection : public Connection, public loop::MsgLoopRun {
    public:

        static const int MSGID_DATA_ARRIVED = 1;
        static const int MSGID_CONNECT_REQ = 2;
        static const int MSGID_CONNECT_RSP = 3;
        static const int MSGID_HEARTBEAT_REQ = 4;
        static const int MSGID_HEARTBEAT_RSP = 5;

        struct ServerMsg {
            int32_t msg_size;
            int32_t msg_id;
        };

        struct ShmemHead {
            int32_t send_size;
            int32_t send_offset;
            int32_t recv_size;
            int32_t recv_offset;
        };

        struct ConnectReq : ServerMsg {
            char shmem_name[128];
#ifdef _WIN32
            char evt_send[128];
            char evt_recv[128];
#endif
        };

        struct ConnectRsp : ServerMsg {
            int32_t conn_id;
        };

        struct HeartBeatReq : ServerMsg {
            char client_shmem_name[128];
        };

        struct HeartBeatRsp : ServerMsg {
            int32_t conn_id;
        };

        IpcConnection();

        virtual ~IpcConnection();

        virtual bool connect(const std::string& addr, Connection_Callback* callback) override;
        virtual void reconnect() override;
        virtual void close() override;
        virtual void send(const char* data, size_t size) override;
        virtual void send(const std::string& data) override;

    private:
        void main_run();
        void do_send();
        bool do_connect();
        void do_recv();
        void do_send_heartbeat();
        void do_close();

        void recv_run();

    private:
        string                      m_addr;
        mutex                       m_send_mtx;
        Connection_Callback*        m_callback;
        volatile bool               m_should_exit;
        bool                        m_connected;
        int32_t                     m_conn_id;
        myutils::FileMapping*       m_shmem;
        ShmemQueue*                 m_recv_queue;
        ShmemQueue*                 m_send_queue;
        Pipe*                       m_pipe;
        thread*                     m_recv_thread;
#ifdef _WIN32
        HANDLE m_hSendEvt;
        HANDLE m_hRecvEvt;
#endif
    };

}

#endif
