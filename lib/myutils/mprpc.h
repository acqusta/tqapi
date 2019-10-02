#ifndef _MPRPC_H
#define _MPRPC_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <msgpack.h>
#include <unordered_map>
#include <vector>
#include "myutils/connection.h"
#include "myutils/loop/MsgRunLoop.h"
#include "myutils/loop/MsgLoopFuture.h"

namespace mprpc {

    using namespace std;
    using namespace std::chrono;
    using namespace loop;

    class MsgPackPacker {
        MsgPackPacker(const MsgPackPacker& copy) {
        }

    public:
        msgpack_sbuffer sb;
        msgpack_packer pk;

        MsgPackPacker() {
            msgpack_sbuffer_init(&sb);
            msgpack_packer_init(&pk, &sb, msgpack_sbuffer_write);
        }
        ~MsgPackPacker() {
            msgpack_sbuffer_destroy(&sb);
        }

        inline void pack_int  (int     v)  { msgpack_pack_int(&pk, v); }
        inline void pack_int8 (int8_t  v)  { msgpack_pack_int8(&pk, v); }
        inline void pack_int16(int16_t v)  { msgpack_pack_int16(&pk, v); }
        inline void pack_int32(int32_t v)  { msgpack_pack_int32(&pk, v); }
        inline void pack_int64(int64_t v)  { msgpack_pack_int64(&pk, v); }
        inline void pack_uint8 (uint8_t  v){ msgpack_pack_uint8(&pk, v); }
        inline void pack_uint16(uint16_t v){ msgpack_pack_uint16(&pk, v); }
        inline void pack_uint32(uint32_t v){ msgpack_pack_uint32(&pk, v); }
        inline void pack_uint64(uint64_t v){ msgpack_pack_uint64(&pk, v); }

        inline void pack_double(double v)  { msgpack_pack_double(&pk, v); }
        inline void pack_array(size_t size){ msgpack_pack_array(&pk, size); }
        inline void pack_map(size_t size)  { msgpack_pack_map(&pk, size); }
        inline void pack_bool(bool v)      { if (v) msgpack_pack_true(&pk); else msgpack_pack_false(&pk); }

        inline void pack_string(const string& str) { pack_string(str.c_str(), str.size()); }

        inline void pack_obj(msgpack_object& obj) { msgpack_pack_object(&pk, obj); }

        inline void pack_bin (const char* data, int len) {
            msgpack_pack_bin(&pk, len);
            msgpack_pack_bin_body(&pk, data, len);
        }

        inline void pack_string(const char* str, size_t len = 0)  { 
            if (len == 0) len = strlen(str);
            msgpack_pack_str(&pk, len);
            msgpack_pack_str_body(&pk, str, len); 
        }

        inline void pack_map_item(const char* key, int64_t val)
        {
            pack_string(key);
            pack_int64( val);
        }

        inline void pack_map_item(const char* key, uint32_t val)
        {
            pack_string(key);
            pack_uint32 (val);
        }

        inline void pack_map_item(const char* key, uint64_t val)
        {
            pack_string(key);
            pack_uint64(val);
        }

        inline void pack_map_item(const char* key, int32_t val)
        {
            pack_string(key);
            pack_int32(val);
        }

        inline void pack_map_item(const char* key, bool val)
        {
            pack_string(key);
            pack_bool(val);
        }

        inline void pack_map_item(const char* key, double val)
        {
            pack_string(key);
            pack_double(val);
        }
        inline void pack_map_item(const char* key, const string& str)
        {
            pack_string(key);
            pack_string(str);
        }
        inline void pack_map_item(const char* key, const char* str)
        {
            pack_string(key);
            pack_string(str);
        }
        inline void pack_map_item_obj(const char* key, const char* obj, size_t obj_len)
        {
            pack_string(key);
            msgpack_pack_str_body(&pk, obj, obj_len);
        }

        inline void pack_map_item_bin(const char* key, const char* bin, int bin_len)
        {
            pack_string(key);
            pack_bin(bin, bin_len);
        }

    };

    static inline bool is_map(msgpack_object& o)    { return o.type == MSGPACK_OBJECT_MAP; }

    static inline bool is_nil(msgpack_object& obj)  {  return obj.type == MSGPACK_OBJECT_NIL;   }

    static inline bool is_bin(msgpack_object& obj)  {   return obj.type == MSGPACK_OBJECT_BIN;  }

    static inline bool is_arr(msgpack_object& obj)  { return obj.type == MSGPACK_OBJECT_ARRAY; }

        static inline bool mp_get(msgpack_object& o, string* v)
    {
        if (o.type != MSGPACK_OBJECT_STR) return false;
        v->assign(o.via.str.ptr, o.via.str.size);
        return true;
    }

    static inline bool mp_get(msgpack_object& o, int64_t* v)
    {
        if (o.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
            *v = o.via.u64;
            return true;
        }
        else if (o.type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
            *v = o.via.i64;
            return true;
        }
        else {
            return false;
        }
    }

    static inline bool mp_get(msgpack_object& o, uint64_t* v)
    {
        return mp_get(o, (int64_t*)v);
    }
    
    static inline bool mp_get(msgpack_object& o, int32_t* v)
    {
        int64_t v2;
        if (mp_get(o, &v2)) {
            *v = (int32_t)v2;
            return true;
        }
        else {
            return false;
        }
    }

    static inline bool mp_get(msgpack_object& o, uint32_t* v)
    {
        return mp_get(o, (int32_t*)v);
    }

    static inline bool mp_get(msgpack_object& o, bool* v)
    {
        if (o.type == MSGPACK_OBJECT_BOOLEAN) {
            *v = o.via.boolean;
            return true;
        }
        else {
            return false;
        }
    }

    static inline bool mp_get(msgpack_object& o, double* v)
    {
        if (o.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
            *v = (double)o.via.u64;
            return true;
        }
        else if (o.type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
            *v = (double)o.via.i64;
            return true;
        }
        else if (o.type == MSGPACK_OBJECT_FLOAT ||
                 o.type == MSGPACK_OBJECT_FLOAT32 ||
                 o.type == MSGPACK_OBJECT_FLOAT64)
        {
            *v = o.via.f64;
            return true;
        }
        else {
            return false;
        }
    }

    static inline bool mp_get(msgpack_object& o, msgpack_object* v)
    {
        *v = o;
        return true;
    }

    template<typename T>
    static inline bool mp_map_get(msgpack_object& o, const char* key, T* v)
    {
        if (!is_map(o)) return false;

        msgpack_object_kv* p = o.via.map.ptr;
        msgpack_object_kv* p_end = p + o.via.map.size;
        for (; p < p_end; p++) {
            if (p->key.type != MSGPACK_OBJECT_STR) continue;
            string str(p->key.via.str.ptr, p->key.via.str.size);
            if (str == key)
                return mp_get(p->val, v);
        }
        return false;
    }

    struct MpRpcError{
        int    error;
        string message;
        string data;
        MpRpcError(int aerr, const string& msg) :
            error(aerr),
            message(msg){}
    };

    struct MpRpcMessage {
        string                      method;
        msgpack_object              params;
        msgpack_object              result;
        msgpack_zone                mp_zone;
        msgpack_object              root;

        int                         err_code;
        string                      err_msg;
        int                         id;
        system_clock::time_point    recv_time;

        string _recv_data;

        MpRpcMessage()
            : err_code(0)
            , id(0)
        {
            msgpack_zone_init(&mp_zone, 2048);
            params.type = MSGPACK_OBJECT_NIL;
            result.type = MSGPACK_OBJECT_NIL;
        }

        ~MpRpcMessage() {
            msgpack_zone_destroy(&mp_zone);
        }

        static shared_ptr<MpRpcMessage> parse(const char* data, size_t size);
    };

    class MpRpcClient_Callback {
    public:
        virtual ~MpRpcClient_Callback() {}
        virtual void on_connected    () {}
        virtual void on_disconnected () {}
        virtual void on_notification (shared_ptr<MpRpcMessage> rpcmsg) {}
        //virtual void on_call_result  (int callid, shared_ptr<MpRpcMessage> cr) {}
    };

    class MpRpcClient : public ::Connection_Callback, public loop::MsgLoopRun {

    public:
        MpRpcClient(shared_ptr<Connection> conn);
        
        virtual ~MpRpcClient();

        bool connect(const string& addr, MpRpcClient_Callback* callback, int timeout=1000);

        void close();

        shared_ptr<MpRpcMessage> call(const char* method, const char* params, size_t params_size, int timeout = 6000);
        
        Future<MpRpcMessage, std::pair<int,string>>  asnyc_call(const char* method, const char* params, size_t params_size, int timeout=6000);

    private:
        void do_send_heartbeat();

        // Connection_Callback
        virtual void on_recv(const char* data, size_t size) override;
        virtual void on_conn_status(bool connected) override;
        virtual void on_idle() override  { }
        
        void on_check_timer();

    private:
        typedef Promise<MpRpcMessage, pair<int, string>> AsyncCallPromise;   
        struct OnRspHandler {
            shared_ptr<AsyncCallPromise>  promise;
            system_clock::time_point      dead_time;

            OnRspHandler() { }

            OnRspHandler(shared_ptr<AsyncCallPromise> p, system_clock::time_point t)
                : promise(p), dead_time(t)
            {}
        };

        string                      m_addr;
        shared_ptr<Connection>      m_conn;
        MpRpcClient_Callback*       m_callback;
        volatile bool               m_connected;
        system_clock::time_point    m_last_hb_rsp_time;
        atomic<int>                 m_cur_callid;
        system_clock::time_point    m_last_hb_time;
        unordered_map<int, OnRspHandler> m_on_rsp_map;
    };

    class ClientConnection {
    public:
        virtual ~ClientConnection() {}
        virtual string id() = 0;
        virtual int max_raw_size() = 0;
        virtual bool send(const char* data, size_t size) = 0;
    };

    class MpRpcServer_Callback {
    public:
        virtual void on_call  (shared_ptr<ClientConnection> connection, shared_ptr<MpRpcMessage> req) = 0;
        virtual void on_close (shared_ptr<ClientConnection> connection) = 0;
    };


    class MpRpcServer {
    public:
        MpRpcServer(MpRpcServer_Callback* callback) : m_callback(callback) {}

        void on_recv(shared_ptr<ClientConnection> connection, const char* data, size_t len);

        void on_close(shared_ptr<ClientConnection> connection);

        bool notify(const string& conn_id, const char* method, const void* data, size_t size);

        bool send(shared_ptr<ClientConnection> connection, const void* data, size_t len);


        bool send_error_rsp(shared_ptr<mprpc::ClientConnection> connection,
                            shared_ptr<mprpc::MpRpcMessage> req,
                            int error,
                            const string& err_msg);
        bool send_rsp(shared_ptr<mprpc::ClientConnection> connection,
                            shared_ptr<mprpc::MpRpcMessage> req,
                            const void* data,
                            size_t size);

        void close(const string& conn_id);

    private:
        MpRpcServer_Callback* m_callback;
        recursive_mutex       m_conn_map_lock;
        unordered_map<string, shared_ptr<ClientConnection>> m_conn_map;
    };
}

#endif
