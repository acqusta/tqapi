#include <assert.h>
#include <glog/logging.h>

#include "myutils/zmq.hpp"
#include "jsonrpc.h"
#include "snappy/snappy.h"

namespace jsonrpc {

    using namespace std;
    using namespace std::chrono;
    using namespace zmq;


    static shared_ptr<JsonRpcMessage> parse(const char* utf8_buf, size_t len)
    {
        auto rpcmsg = make_shared<JsonRpcMessage>();

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
                        LOG(INFO) << "rsp_time: " << rpcmsg->method << " "
                            << (send_time - recv_time) << " "
                            << time_point_cast<microseconds>(system_clock::now()).time_since_epoch().count() - recv_time;
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


    static int my_rand()
    {
        static std::default_random_engine         g_generator;
        static std::uniform_int_distribution<int> g_distribution(1000000, INT32_MAX);;
        static bool inited = false;
        if (!inited) {
            inited = true;
            int32_t seed = system_clock::now().time_since_epoch().count() % INT32_MAX;
            g_generator.seed(seed);
        }

        return g_distribution(g_generator);
    }

    JsonRpcClient::JsonRpcClient()
        : m_remote_sock(nullptr)
        , m_callback(nullptr)
        , m_should_exit(false)
        , m_connected(false)
    {
        m_cur_callid = 0;

        m_pull_sock = new zmq::socket_t(m_zmq_ctx, ZMQ_PULL);
        m_pull_sock->bind("inproc://pull_sock");
        m_push_sock = new zmq::socket_t(m_zmq_ctx, ZMQ_PUSH);
        m_push_sock->connect("inproc://pull_sock");

        m_main_thread = new thread(&JsonRpcClient::main_run, this);
        m_callback_thread = new thread(&JsonRpcClient::callback_run, this);

    }

    JsonRpcClient::~JsonRpcClient()
    {
        m_should_exit = true;

        m_main_thread->join();
        m_callback_thread->join();

        delete m_main_thread;
        delete m_callback_thread;

        delete m_pull_sock;
        delete m_push_sock;
        if (m_remote_sock) delete m_remote_sock;
    }

    void JsonRpcClient::callback_run()
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

    void JsonRpcClient::call_callback(function<void()> func)
    {
        unique_lock<mutex> lock(m_asyncall_lock);
        m_asyncall_queue.push_back(func);
    }

    void JsonRpcClient::main_run()
    {
        auto last_hb_time = system_clock::now();

        zmq_pollitem_t items[2];
        memset(items, 0, sizeof(items));
        int items_count = 1;
        items[0].socket = (void*)*m_pull_sock;
        items[0].events = ZMQ_POLLIN;

        while (!m_should_exit) {

            if (m_remote_sock && items_count == 1) {
                items[1].socket = (void*)*m_remote_sock;
                items[1].events = ZMQ_POLLIN;
                items_count = 2;
            }

            zmq::poll(items, items_count, 100);

            if (items[0].revents & ZMQ_POLLIN) {
                zmq::message_t msg;
                if (m_pull_sock->recv(&msg)) {
                    const char* p = (const char*)msg.data();
                    switch (p[0]){
                    case 'S':   do_send(p + 1, msg.size() - 1); break;
                    case 'C':   do_connect(); break;
                    }
                }
            }

            if (items_count == 2 ) {
                if (items[1].revents & ZMQ_POLLIN)  do_recv();

                auto now = system_clock::now();
                if ( now - last_hb_time > seconds(1)) {
                    last_hb_time = now;
                    do_send_heartbeat();
                }

                if (m_connected) {
                    if (now - m_last_hb_rsp_time > seconds(3)) {
                        m_connected = false;
                        if (m_callback)  call_callback(bind(&JsonRpcClient_Callback::on_disconnected, m_callback));
                    }
                }
            }
        }

    }

    void JsonRpcClient::do_recv()
    {
        try {
            zmq::message_t msg;
            if (!m_remote_sock->recv(&msg, ZMQ_DONTWAIT)) return;

            shared_ptr<JsonRpcMessage> rpcmsg = parse((const char*)msg.data(), msg.size());
            if (!rpcmsg) return;

            if (rpcmsg->method == ".sys.heartbeat") {
                m_last_hb_rsp_time = system_clock::now();
                if (!m_connected) {
                    m_connected = true;
                    if (m_callback)
                        call_callback(bind(&JsonRpcClient_Callback::on_connected, m_callback));
                }
                if (m_callback)
                    call_callback(bind(&JsonRpcClient_Callback::on_notification, m_callback, rpcmsg));
            }
            else if (rpcmsg->id) {
                unique_lock<mutex> lock(m_waiter_map_lock);

                auto it = m_waiter_map.find(rpcmsg->id);
                if (it != m_waiter_map.end()) {
                    it->second->result = rpcmsg;
                    it->second->cond.notify_all();
                }
                else {
                    call_callback(bind(&JsonRpcClient_Callback::on_call_result, m_callback, rpcmsg->id, rpcmsg));
                }
            }
            else {
                // Notification message
                if (m_callback && !rpcmsg->method.empty())
                    call_callback(bind(&JsonRpcClient_Callback::on_notification, m_callback, rpcmsg));
            }
        }
        catch (exception& e) {
            std::cerr << e.what() << endl;
        }
    }

    bool JsonRpcClient::connect(const string& addr, JsonRpcClient_Callback* callback)
    {
        if (addr.empty()) return false;

        m_addr = addr;
        m_callback = callback;
        m_push_sock->send("C", 1);
        return true;
    }

    void JsonRpcClient::do_connect() {

        LOG(INFO) << "Connect to " << m_addr;
        assert(m_remote_sock == nullptr);
        assert(!m_addr.empty());

        m_remote_sock = new zmq::socket_t(m_zmq_ctx, ZMQ_DEALER);
        m_remote_sock->setsockopt(ZMQ_RCVTIMEO, 2000);
        m_remote_sock->setsockopt(ZMQ_SNDTIMEO, 2000);
        m_remote_sock->setsockopt(ZMQ_LINGER, 0);
        
        stringstream ss;
        ss << my_rand() << "$" << my_rand();
        string id = ss.str();
        m_remote_sock->setsockopt(ZMQ_IDENTITY, id.c_str(), id.size());

        m_remote_sock->connect(m_addr);
    }

    void JsonRpcClient::close()
    {
        m_should_exit = true;
    }

    int JsonRpcClient::asnyc_call(const char* method, const char* params, size_t params_size)
    {
        int callid = ++m_cur_callid;

        MsgPackPacker pk;
        pk.pack_map(3);
        pk.pack_map_item    ("id",     callid);
        pk.pack_map_item    ("method", method);
        pk.pack_map_item_obj("params", params, params_size);

        stringstream ss;
        ss << "S" << string(pk.sb.data, pk.sb.size);
        string cmd = ss.str();

        m_push_sock->send(cmd.c_str(), cmd.size());
        return callid;
    }

    static inline shared_ptr<JsonRpcMessage> build_timeout_result(const char* method, int callid)
    {
        auto rsp = make_shared<JsonRpcMessage>();

        rsp->err_code = -1;
        rsp->err_msg = "rpc_call timeout";
        rsp->id = callid;
        rsp->method = method;

        return rsp;
    }

    shared_ptr<JsonRpcMessage> JsonRpcClient::call(const char* method, const char* params, size_t params_size, int timeout)
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

        stringstream ss;
        ss << "S" << string(pk.sb.data, pk.sb.size);
        string cmd = ss.str();

        m_push_sock->send(cmd.c_str(), cmd.size());
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

    void JsonRpcClient::do_send(const char* buf, size_t size)
    {
        assert(m_remote_sock);
        try {
            m_remote_sock->send(buf, size);
        }
        catch (exception& e) {
            cerr << "do_send failed: " << e.what() << endl;
        }
    }

    void JsonRpcClient::do_send_heartbeat()
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

        do_send(pk.sb.data, pk.sb.size);
    }


    ZmqJsonRpcServer::ZmqJsonRpcServer(JsonRpcServer* server) :
        m_server(server),
        m_remote_sock(nullptr),
        m_should_exit(false),
        m_connected(false)
    {
        m_pull_sock = new zmq::socket_t(m_zmq_ctx, ZMQ_PULL);
        m_pull_sock->bind("inproc://pull_sock");
        m_push_sock = new zmq::socket_t(m_zmq_ctx, ZMQ_PUSH);
        m_push_sock->connect("inproc://pull_sock");

        m_main_thread = new thread(&ZmqJsonRpcServer::main_run, this);
    }

    ZmqJsonRpcServer::~ZmqJsonRpcServer()
    {
        m_should_exit = true;

        m_main_thread->join();

        delete m_main_thread;

        delete m_pull_sock;
        delete m_push_sock;
        if (m_remote_sock) delete m_remote_sock;
    }

    bool ZmqJsonRpcServer::listen(const string& addr)
    {
        if (addr.empty() || m_remote_sock) return false;

        m_addr = addr;

        m_push_sock->send("L",1);
        return true;
    }

    void ZmqJsonRpcServer::do_listen()
    {
        if (m_remote_sock) return;

        try {
            VLOG(1) << "JsonRpc listen at " << m_addr;
            m_remote_sock = new zmq::socket_t(m_zmq_ctx, ZMQ_ROUTER);
            m_remote_sock->setsockopt(ZMQ_RCVTIMEO, 2000);
            m_remote_sock->setsockopt(ZMQ_SNDTIMEO, 2000);
            m_remote_sock->setsockopt(ZMQ_LINGER, 0);
            m_remote_sock->bind(m_addr);
        }
        catch (...) {
            LOG(FATAL) << "Listen error: " << ::zmq_strerror(::zmq_errno());
        }
    }

    void ZmqJsonRpcServer::close()
    {
        m_push_sock->send("C", 1);
    }

    void ZmqJsonRpcServer::main_run()
    {
        zmq_pollitem_t items[2];
        memset(items, 0, sizeof(items));
        int items_count = 1;
        items[0].socket = (void*)*m_pull_sock;
        items[0].events = ZMQ_POLLIN;

        while (!m_should_exit) {

            if (m_remote_sock && items_count == 1) {
                items[1].socket = (void*)*m_remote_sock;
                items[1].events = ZMQ_POLLIN;
                items_count = 2;
            }

            zmq::poll(items, items_count, 100);

            if (items[0].revents & ZMQ_POLLIN) {
                zmq::message_t msg1, msg2;
                if (m_pull_sock->recv(&msg1)) {
                    if (msg1.more() && m_pull_sock->recv(&msg2)) {
                        if (m_remote_sock)
                            do_send(string((const char*)msg1.data(), msg1.size()), msg2);
                    }
                    else {
                        const char* p = (const char*)msg1.data();
                        switch (p[0]){
                        case 'L':   do_listen(); break;
                        case 'C':   break;
                        }
                    }
                }
            }

            if (items_count == 2 && items[1].revents & ZMQ_POLLIN) {
                do_recv();
            }
        }

    }

    void ZmqJsonRpcServer::do_send(const string& client, zmq::message_t& data)
    {
        //VLOG(1) << "Send " << string((const char*)data.data(), data.size());
        //VLOG(1) << "Send size=" << data.size();

        auto it = m_client_map.find(client);
        if (it == m_client_map.end())
            return;
        try {
            auto& conn = it->second;
            m_remote_sock->send(conn->m_addr.c_str(), conn->m_addr.size(), ZMQ_SNDMORE);
            m_remote_sock->send(data);
        }
        catch (exception& e) {
            LOG(ERROR) << "do_send failed: " << e.what();
        }
    }

    //void ZmqJsonRpcServer::do_send(const string& id, const char* data, size_t size)
    //{
    //    auto it = m_client_map.find(id);
    //    if (it == m_client_map.end())
    //        return;
    //    try {
    //        auto& conn = it->second;
    //        m_remote_sock->send(conn->m_addr.c_str(), conn->m_addr.size(), ZMQ_SNDMORE);
    //        m_remote_sock->send(data, size);
    //    }
    //    catch (exception& e) {
    //        LOG(ERROR) << "do_send failed: " << e.what();
    //    }
    //}

    void ZmqJsonRpcServer::do_recv()
    {
        zmq::message_t addr, body;

        if (!m_remote_sock->recv(&addr, ZMQ_DONTWAIT)) return;
        if (!addr.more() || !m_remote_sock->recv(&body, ZMQ_DONTWAIT)) return;

        //VLOG(1) << "Recv " << string((const char*)addr.data(), addr.size()) << "," << string((const char*)body.data(), body.size());
        //VLOG(1) << "Recv size =" << body.size();

        string id;
        string addr_str((const char*)addr.data(), addr.size());
        const char* p = strchr(addr_str.c_str(), '$');
        if (p)
            id = string(addr_str.c_str(), p - 1);
        else
            id = addr_str;

        shared_ptr<ZmqJsonRpcServer_Connection> conn;
        auto it = m_client_map.find(id);
        if (it != m_client_map.end()) {
            it->second->set_addr(addr_str);
            conn = it->second;
        }
        else {
            conn = make_shared<ZmqJsonRpcServer_Connection > (id, addr_str, this);
            m_client_map[id] = conn;
        }

        this->m_server->on_recv(conn, (const char*)body.data(), body.size());
    }

    bool ZmqJsonRpcServer::send(const string& id, const char* data, size_t size)
    {
        unique_lock<mutex> lock(m_send_lock);
        if (size < 20000) {
            m_push_sock->send(id.c_str(), id.size(), ZMQ_SNDMORE);
            m_push_sock->send(data, size);
        }
        else {
            size_t len = snappy::MaxCompressedLength(size);
            char* buf = new char[5 + len];
            buf[0] = 'S';
            *(uint32_t*)(buf + 1) = size;
            snappy::RawCompress(data, size, buf + 5, &len);
            m_push_sock->send(id.c_str(), id.size(), ZMQ_SNDMORE);
            m_push_sock->send(buf, len + 5);
            delete[] buf;
        }

        return true;
    }

    void JsonRpcServer::on_recv(shared_ptr<Connection> connection, const char* data, size_t size)
    {
        auto rpcmsg = parse(data, size);
        if (!rpcmsg ||
            //rpcmsg->jsonrpc != "2.0" ||
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

    bool JsonRpcServer::send(shared_ptr<Connection> connection, const void* data, size_t len)
    {
        return connection->send((const char*)data, len);
    }

    void JsonRpcServer::on_close(shared_ptr<Connection> connection)
    {
        return m_callback->on_close(connection);
    }

    
}
