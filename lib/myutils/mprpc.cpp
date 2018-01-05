#include <assert.h>

#include "mprpc.h"
#include "snappy/snappy.h"

namespace mprpc {

    using namespace std;
    using namespace std::chrono;

    shared_ptr<MpRpcMessage> MpRpcMessage::parse(const char* utf8_buf, size_t len)
    {
        auto rpcmsg = make_shared<MpRpcMessage>();

        try {
            if (utf8_buf[0] == 'S') {
                uint32_t orig_len = *(uint32_t*)(utf8_buf + 1);
                if (!snappy::Uncompress(utf8_buf + 5, len - 5, &rpcmsg->_recv_data))
                    return nullptr;
            }
            else {
                rpcmsg->_recv_data.assign(utf8_buf, len);
            }
            msgpack_unpack_return ret = msgpack_unpack(rpcmsg->_recv_data.c_str(), rpcmsg->_recv_data.size(),
                                                       nullptr, &rpcmsg->mp_zone, &rpcmsg->root);
            if ( ret != MSGPACK_UNPACK_SUCCESS)
                return nullptr;

            if (!is_map(rpcmsg->root)) return nullptr;

            for (uint32_t i = 0; i < rpcmsg->root.via.map.size; i++) {
                auto o = rpcmsg->root.via.map.ptr + i;
                if (o->key.type != MSGPACK_OBJECT_STR) continue;
                const string str(o->key.via.str.ptr, o->key.via.str.size);
                if (str == "id") {
                    if (o->val.type == MSGPACK_OBJECT_NEGATIVE_INTEGER)
                        rpcmsg->id = (int)o->val.via.i64;
                    else if (o->val.type == MSGPACK_OBJECT_POSITIVE_INTEGER)
                        rpcmsg->id = (int)o->val.via.u64;
                }
                else if (str == "method") {
                    if (o->val.type == MSGPACK_OBJECT_STR)
                        rpcmsg->method.assign(o->val.via.str.ptr, o->val.via.str.size);
                }
                else if (str == "params") {
                    rpcmsg->params = o->val;
                }
                else if (str == "result") {
                    rpcmsg->result = o->val;
                }
                else if (str == "error") {
                    if (o->val.type == MSGPACK_OBJECT_MAP) {
                        for (uint32_t n = 0; n < o->val.via.map.size; n++) {
                            auto p = o->val.via.map.ptr + n;
                            if (p->key.type == MSGPACK_OBJECT_STR && memcmp(p->key.via.str.ptr, "code", 4) == 0) {
                                if (p->val.type == MSGPACK_OBJECT_NEGATIVE_INTEGER)
                                    rpcmsg->err_code = (int)p->val.via.i64;
                                else if (p->val.type == MSGPACK_OBJECT_POSITIVE_INTEGER)
                                    rpcmsg->err_code = (int)p->val.via.u64;
                            }
                            else if (p->key.type == MSGPACK_OBJECT_STR && memcmp(p->key.via.str.ptr, "message", 7) == 0) {
                                if (p->val.type == MSGPACK_OBJECT_STR)
                                    rpcmsg->err_msg.assign(p->val.via.str.ptr, p->val.via.str.size);
                            }
                        }
                    }
                }
#if 0
                else if (str == "debug") {
                    if (o->val.type == MSGPACK_OBJECT_MAP) {
                        uint64_t recv_time = 0;
                        uint64_t send_time = 0;
                        for (uint32_t n = 0; n < o->val.via.map.size; n++) {
                            auto p = o->val.via.map.ptr + n;
                            if (p->key.type == MSGPACK_OBJECT_STR) {
                                if (memcmp(p->key.via.str.ptr, "recv_time", 9) == 0)
                                    recv_time = p->val.via.u64;
                                else if (memcmp(p->key.via.str.ptr, "send_time", 9) == 0)
                                    send_time = p->val.via.u64;
                            }
                        }
                        std::cout << "rsp_time: " << rpcmsg->method << " "
                            << (send_time - recv_time) << " "
                            << time_point_cast<microseconds>(system_clock::now()).time_since_epoch().count() - recv_time
                            << endl;
                    }
                }
#endif
            }
            return rpcmsg;
        }
        catch (exception& e) {
            cerr << "parse conf failure: " << e.what();
            return nullptr;
        }
    }

    MpRpcClient::MpRpcClient(shared_ptr<Connection> conn)
        : m_conn(conn)
        , m_callback(nullptr)
        , m_should_exit(false)
        , m_connected(false)
        , m_last_hb_time(system_clock::time_point())
    {
        m_cur_callid = 0;

        m_callback_thread = new thread(&MpRpcClient::callback_run, this);
    }

    MpRpcClient::~MpRpcClient()
    {
        m_should_exit = true;

        m_callback_thread->join();

        delete m_callback_thread;
    }

    void MpRpcClient::callback_run()
    {
        while (!m_should_exit) {
            function<void()> func;
            {
                unique_lock<mutex> lock(m_asyncall_lock);
                if (!m_asyncall_queue.empty()) {
                    func = m_asyncall_queue.front();
                    m_asyncall_queue.pop_front();
                }
                else {
                    m_asyncall_cond.wait_for(lock, milliseconds(100));
                    continue;
                }
            }
            if (func) func();
        }
    }

    void MpRpcClient::call_callback(function<void()> func)
    {
        unique_lock<mutex> lock(m_asyncall_lock);
        m_asyncall_queue.push_back(func);
        m_asyncall_cond.notify_one();
    }

    void MpRpcClient::on_idle()
    {
        auto now = system_clock::now();
        if (now - m_last_hb_time > seconds(2)) {
            m_last_hb_time = now;
            do_send_heartbeat();
        }

        if (m_connected) {
            if (now - m_last_hb_rsp_time > seconds(3)) {
                m_connected = false;
                if (m_callback)  call_callback(bind(&MpRpcClient_Callback::on_disconnected, m_callback));
            }
        }
    }

    void MpRpcClient::on_conn_status(bool connected)
    {
        m_connected = connected;
        if (m_callback) {
            if (connected)
                call_callback(bind(&MpRpcClient_Callback::on_connected, m_callback));
            else
                call_callback(bind(&MpRpcClient_Callback::on_disconnected, m_callback));
        }
    }

    void MpRpcClient::on_recv(const char* data, size_t size)
    {
        try {
            shared_ptr<MpRpcMessage> rpcmsg = MpRpcMessage::parse(data, size);
            if (!rpcmsg) return;

            rpcmsg->recv_time = system_clock::now();

            if (rpcmsg->method == ".sys.heartbeat") {
                m_last_hb_rsp_time = system_clock::now();
                if (!m_connected) {
                    m_connected = true;
                    if (m_callback)
                        call_callback(bind(&MpRpcClient_Callback::on_connected, m_callback));
                }
                if (m_callback)
                    call_callback(bind(&MpRpcClient_Callback::on_notification, m_callback, rpcmsg));
            }
            else if (rpcmsg->id) {
                unique_lock<mutex> lock(m_waiter_map_lock);

                auto it = m_waiter_map.find(rpcmsg->id);
                if (it != m_waiter_map.end()) {
                    it->second->result = rpcmsg;
                    it->second->cond.notify_all();
                }
                else {
                    call_callback(bind(&MpRpcClient_Callback::on_call_result, m_callback, rpcmsg->id, rpcmsg));
                }
            }
            else {
                // Notification message
                if (m_callback && !rpcmsg->method.empty())
                    call_callback(bind(&MpRpcClient_Callback::on_notification, m_callback, rpcmsg));
            }
        }
        catch (exception& e) {
            std::cerr << e.what() << endl;
        }
    }

    bool MpRpcClient::connect(const string& addr, MpRpcClient_Callback* callback)
    {
        if (addr.empty()) return false;

        m_addr = addr;
        m_callback = callback;

        m_conn->connect(addr, this);
        return true;
    }

    void MpRpcClient::close()
    {
        m_conn->close();
        m_should_exit = true;
    }

    int MpRpcClient::asnyc_call(const char* method, const char* params, size_t params_size)
    {
        int callid = ++m_cur_callid;

        MsgPackPacker pk;
        pk.pack_map(3);
        pk.pack_map_item    ("id",     callid);
        pk.pack_map_item    ("method", method);
        pk.pack_map_item_obj("params", params, params_size);

        m_conn->send(pk.sb.data, pk.sb.size);
        return callid;
    }

    static inline shared_ptr<MpRpcMessage> build_timeout_result(const char* method, int callid)
    {
        auto rsp = make_shared<MpRpcMessage>();

        rsp->err_code = -1;
        rsp->err_msg = "rpc_call timeout";
        rsp->id = callid;
        rsp->method = method;

        return rsp;
    }

    shared_ptr<MpRpcMessage> MpRpcClient::call(const char* method, const char* params, size_t params_size, int timeout)
    {
        int callid = ++m_cur_callid;

        ResultWaiter waiter;
        if (timeout != 0) {
            unique_lock<mutex> lock(m_waiter_map_lock);
            m_waiter_map[callid] = &waiter;
        }

        MsgPackPacker pk;
        pk.pack_map(3);
        pk.pack_map_item    ("id", callid);
        pk.pack_map_item    ("method", method);
        pk.pack_map_item_obj("params", params, params_size);

        m_conn->send(pk.sb.data, pk.sb.size);

        if (!timeout)
            return nullptr;

        auto dead_time = system_clock::now() + milliseconds(timeout);
        while (true) {
            unique_lock<mutex> lock(m_waiter_map_lock);
            waiter.cond.wait_for(lock, dead_time - system_clock::now());
            if (waiter.result || system_clock::now() >= dead_time){
                m_waiter_map.erase(m_waiter_map.find(callid));
                break;
            }
        }

        return waiter.result ? waiter.result : build_timeout_result(method, callid);
    }

    void MpRpcClient::do_send_heartbeat()
    {
        int callid = ++m_cur_callid;

        MsgPackPacker pk_params;
        pk_params.pack_map(1);
        pk_params.pack_map_item("time", system_clock::now().time_since_epoch().count());
         
        MsgPackPacker pk;
        pk.pack_map(3);
        pk.pack_map_item    ("id",     callid);
        pk.pack_map_item    ("method", ".sys.heartbeat");
        pk.pack_map_item_obj("params", pk_params.sb.data, pk_params.sb.size);

        m_conn->send(pk.sb.data, pk.sb.size);
    }


    void MpRpcServer::on_recv(shared_ptr<ClientConnection> connection, const char* data, size_t size)
    {
        auto rpcmsg = MpRpcMessage::parse(data, size);
        if (!rpcmsg ||
            rpcmsg->id == 0 ||
            rpcmsg->method.empty())
        {
            return;
        }

        rpcmsg->recv_time = system_clock::now();

        if (!rpcmsg->method.empty()) {
            m_callback->on_call(connection, rpcmsg);
        }
        else {
            MsgPackPacker pk;
            pk.pack_map(2);
            pk.pack_map_item ("id", rpcmsg->id);
            pk.pack_string   ("error");
            // error -> [ code, message]
            pk.pack_map(2);
            pk.pack_map_item ("code", -1);
            pk.pack_map_item ("message", "empty method");

            connection->send(pk.sb.data, pk.sb.size);
        }
    }

    bool MpRpcServer::send(shared_ptr<ClientConnection> conn, const void* data, size_t size)
    {
        if (size < conn->max_raw_size()) {
            return conn->send((const char*)data, size);
        }
        else {
            //static uint64_t time1;
            //static uint64_t time2;
            //static uint64_t count;
            //auto begin_time = system_clock::now();

            size_t len = snappy::MaxCompressedLength(size);
            char* buf = new char[5 + len];
            buf[0] = 'S';
            *(uint32_t*)(buf + 1) = (uint32_t)size;
            snappy::RawCompress((const char*)data, size, buf + 5, &len);

            //time1 += duration_cast<microseconds>(system_clock::now() - begin_time).count();

            bool r = conn->send (buf, len + 5);
            delete[] buf;

            //time2 += duration_cast<microseconds>(system_clock::now() - begin_time).count();

            //count++;
            //if (count == 20) {
            //    std::cout << "send time: " << (time1 / count) << "," << (time2 / count) << endl;
            //    time1 = time2 = 0;
            //    count = 0;
            //}

            return r;
        }
    }

    void MpRpcServer::on_close(shared_ptr<ClientConnection> connection)
    {
        return m_callback->on_close(connection);
    }
}
