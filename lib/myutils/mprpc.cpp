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
                //uint32_t orig_len = *(uint32_t*)(utf8_buf + 1);
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
        , m_connected(false)
        , m_last_hb_rsp_time(system_clock::now())
        //, m_last_hb_time
    {
        m_cur_callid = 0;

        msg_loop().PostDelayedTask(bind(&MpRpcClient::on_check_timer, this), 100);
    }

    MpRpcClient::~MpRpcClient()
    {
        close();
    }

    void MpRpcClient::on_check_timer()
    {
        msg_loop().PostDelayedTask(bind(&MpRpcClient::on_check_timer, this), 100);

        auto now = system_clock::now();
        if (now - m_last_hb_time > seconds(2)) {
            m_last_hb_time = now;
            do_send_heartbeat();
        }

        if (now - m_last_hb_rsp_time > seconds(4)) {
            if (m_connected) {
                m_connected = false;
                if (m_callback) m_callback->on_disconnected();
            }
            m_last_hb_rsp_time = now;
            m_conn->reconnect();
        }

        for (auto it = m_on_rsp_map.begin(); it != m_on_rsp_map.end(); ) {
            if (it->second.dead_time < now) {
                it->second.promise->set_error(make_shared<pair<int, string>>(-1, "timeout"));
                it = m_on_rsp_map.erase(it);
            }
            else {
                it++;
            }
        }
    }

    void MpRpcClient::on_conn_status(bool connected)
    {
        msg_loop().PostTask([this, connected]() {
            m_connected = connected;

            if (m_connected)
                do_send_heartbeat();

            if (m_callback) {
                if (connected)
                    m_callback->on_connected();
                else
                    m_callback->on_disconnected();
            }
        });
    }

    void MpRpcClient::on_recv(const char* data, size_t size)
    {
        shared_ptr<MpRpcMessage> rpcmsg = MpRpcMessage::parse(data, size);
        if (!rpcmsg) return;
        rpcmsg->recv_time = system_clock::now();

        msg_loop().PostTask([this, rpcmsg]() {
            if (rpcmsg->method == ".sys.heartbeat") {
                m_last_hb_rsp_time = system_clock::now();
                if (!m_connected) {
                    m_connected = true;
                    if (m_callback) m_callback->on_connected();
                }
                if (m_callback) m_callback->on_notification(rpcmsg);
            }
            else if (rpcmsg->id) {
                auto it = m_on_rsp_map.find(rpcmsg->id);
                if (it != m_on_rsp_map.end()) {
                    auto rsp = it->second.promise;
                    m_on_rsp_map.erase(it);
                    rsp->set_value(rpcmsg);
                }
            }
            else {
                // Notification message
                if (m_callback && !rpcmsg->method.empty())
                    m_callback->on_notification(rpcmsg);
            }
        });
    }

    bool MpRpcClient::connect(const string& addr, MpRpcClient_Callback* callback)
    {
        if (addr.empty()) return false;

        m_addr = addr;
        m_conn->connect(addr, this);

        auto begin_time = system_clock::now();
        while (system_clock::now() - begin_time < seconds(1) && !m_connected) {
            // nothing
        }

        m_msg_loop.PostTask([this, callback]() { this->m_callback = callback; });
        return m_callback;
    }

    void MpRpcClient::close()
    {
        close_loop();
        m_conn->close();
    }

    Future<MpRpcMessage, pair<int, string>> MpRpcClient::asnyc_call(const char* method, const char* params, size_t params_size, int timeout_ms)
    {
        int callid = ++m_cur_callid;
        MsgPackPacker pk;
        pk.pack_map(3);
        pk.pack_map_item("id", callid);
        pk.pack_map_item("method", method);
        pk.pack_map_item_obj("params", params, params_size);

        auto msg = make_shared<string>(pk.sb.data, pk.sb.size);
        auto prom = make_shared<AsyncCallPromise>(&this->m_msg_loop);

        this->msg_loop().PostTask([this, prom, callid, timeout_ms, msg]() {
            m_conn->send(msg->data(), msg->size());
            m_on_rsp_map[callid] = OnRspHandler(prom, system_clock::now() + milliseconds(timeout_ms));
        });
        return prom->get_future();
    }

    //static inline shared_ptr<MpRpcMessage> build_timeout_result(const char* method, int callid)
    //{
    //    auto rsp = make_shared<MpRpcMessage>();

    //    rsp->err_code = -1;
    //    rsp->err_msg = "rpc_call timeout";
    //    rsp->id = callid;
    //    rsp->method = method;

    //    return rsp;
    //}

    shared_ptr<MpRpcMessage> MpRpcClient::call(const char* method, const char* params, size_t params_size, int timeout)
    {
        int callid = ++m_cur_callid;

        mutex mtx;
        condition_variable  cond;
        shared_ptr<MpRpcMessage> rsp_msg;

        if (!timeout) {
            asnyc_call(method, params, params_size, timeout);
            return nullptr;
        } else {
            asnyc_call(method, params, params_size, timeout)
                .in_loop(nullptr)
                .on_success([&mtx, &cond, &rsp_msg](shared_ptr<MpRpcMessage> msg) {
                    unique_lock<mutex> lock(mtx);
                    rsp_msg = msg;
                    cond.notify_one();
                })
                .on_failure([callid, method, &mtx, &cond, &rsp_msg](shared_ptr<pair<int, string>> e) {
                    auto rsp = make_shared<MpRpcMessage>();
                    rsp->err_code = e->first;
                    rsp->err_msg = e->second;
                    rsp->id = callid;
                    rsp->method = method;
                    unique_lock<mutex> lock(mtx);
                    rsp_msg = rsp;
                    cond.notify_one();
                });
        
            unique_lock<mutex> lock(mtx);
            cond.wait(lock);
            return rsp_msg;
        }
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


    void MpRpcServer::on_recv(shared_ptr<ClientConnection> conn, const char* data, size_t size)
    {
        auto rpcmsg = MpRpcMessage::parse(data, size);
        if (!rpcmsg ||
            rpcmsg->id == 0 ||
            rpcmsg->method.empty())
        {
            return;
        }

        {
            unique_lock<recursive_mutex> lock(m_conn_map_lock);
            auto it = m_conn_map.find(conn->id());
            if (it == m_conn_map.end()) m_conn_map[conn->id()] = conn;
        }

        rpcmsg->recv_time = system_clock::now();

        if (rpcmsg->method.size()) {
            if (m_callback)
                m_callback->on_call(conn, rpcmsg);
            else {
                send_error_rsp(conn, rpcmsg, -1, "unknown method");
            }
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

            conn->send(pk.sb.data, pk.sb.size);
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

    void MpRpcServer::on_close(shared_ptr<ClientConnection> conn)
    {
        {
            unique_lock<recursive_mutex> lock(m_conn_map_lock);
            auto it = m_conn_map.find(conn->id());
            if (it != m_conn_map.end()) m_conn_map.erase(it);
        }

        if (m_callback)
            m_callback->on_close(conn);
    }

    bool MpRpcServer::send_error_rsp(shared_ptr<mprpc::ClientConnection> conn,
                                    shared_ptr<mprpc::MpRpcMessage> req,
                                    int error,
                                    const string& err_msg)
    {
        MsgPackPacker pk;
        pk.pack_map(4);

        pk.pack_map_item("id", req->id);
        pk.pack_map_item("method", req->method);

        pk.pack_string("error");
        pk.pack_map(2);
        pk.pack_map_item("code", error);
        pk.pack_map_item("message", err_msg);

        pk.pack_string("debug");
        pk.pack_map(2);
        pk.pack_map_item("recv_time", time_point_cast<microseconds>(req->recv_time).time_since_epoch().count());
        pk.pack_map_item("send_time", time_point_cast<microseconds>(system_clock::now()).time_since_epoch().count());

        return send(conn, pk.sb.data, pk.sb.size);
    }

    bool MpRpcServer::send_rsp(shared_ptr<mprpc::ClientConnection> conn,
        shared_ptr<mprpc::MpRpcMessage> req,
        const void* data,
        size_t size)
    {
        MsgPackPacker pk;

        pk.pack_map(4);
        pk.pack_map_item("id", req->id);
        pk.pack_map_item("method", req->method);
        pk.pack_string("result");
        msgpack_pack_str_body(&pk.pk, data, size);

        pk.pack_string("debug");
        pk.pack_map(2);
        pk.pack_map_item("recv_time", time_point_cast<microseconds>(req->recv_time).time_since_epoch().count());
        pk.pack_map_item("send_time", time_point_cast<microseconds>(system_clock::now()).time_since_epoch().count());

        return send(conn, pk.sb.data, pk.sb.size);
    }

    bool MpRpcServer::notify(const string& conn_id, const char* method, const void* data, size_t size)
    {
        MsgPackPacker pk;
        pk.pack_map(3);

        pk.pack_map_item("id", 0);
        pk.pack_map_item("method", method);
        pk.pack_string("params");
        msgpack_pack_str_body(&pk.pk, data, size);

        unique_lock<recursive_mutex> lock(m_conn_map_lock);

        for (auto it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
            if (conn_id.empty()) {
                send(it->second, pk.sb.data, pk.sb.size);
            }
            else if (conn_id == it->first) {
                return send(it->second, pk.sb.data, pk.sb.size);
            }
        }
        return true;
    }

    void MpRpcServer::close(const string& conn_id)
    {
        unique_lock<recursive_mutex> lock(m_conn_map_lock);

        auto it = m_conn_map.find(conn_id);
        if (it != m_conn_map.end()) {
            //auto conn = it->second;
            m_conn_map.erase(it);
        }
    }
}
