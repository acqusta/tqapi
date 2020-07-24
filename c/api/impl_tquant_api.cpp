#include <sstream>
#include <unordered_set>
#ifndef _WIN32
# include <dlfcn.h>
#endif

#include "myutils/stringutils.h"
#include "myutils/mprpc.h"
#include "myutils/socket_connection.h"
#include "myutils/ipc_connection.h"
#include "myutils/socketutils.h"
#include "myutils/misc.h"
#include "tquant_api.h"
#include "impl_tquant_api.h"
#include "impl_data_api.h"
#include "impl_trade_api.h"


namespace tquant { namespace api { namespace impl {

    using namespace ::mprpc;
    using namespace ::tquant::api;
    using namespace ::myutils;

    MpRpc_Connection::MpRpc_Connection()
        : m_callback(nullptr)
        , m_client(nullptr)
    {
    }

    bool MpRpc_Connection::connect(const string& params) {
        string addr = params;
        if (strncmp(addr.c_str(), "tcp://", 6) == 0) {
            auto conn = make_shared<SocketConnection>();
            m_client = new MpRpcClient(conn);
            m_client->connect(addr, this);
            return true;
        }
        else if (strncmp(addr.c_str(), "ipc://", 6) == 0) {
            auto conn = make_shared<IpcConnection>();
            m_client = new MpRpcClient(conn);
            m_client->connect(addr, this);
            return true;
        }
        else {
            throw std::runtime_error("unknown addr");
            return false;
        }
    }

    MpRpc_Connection::~MpRpc_Connection() {
            m_msgloop.close_loop();
            delete m_client;
        }

    void MpRpc_Connection::on_connected() {
        if (m_callback) m_callback->on_connected();
        }

    void MpRpc_Connection::on_disconnected()
    {
        if (m_callback) m_callback->on_disconnected();
    }

    void MpRpc_Connection::on_notification(shared_ptr<MpRpcMessage> rpcmsg)
    {
        m_msgloop.msg_loop().post_task([this, rpcmsg] {
            if (m_callback) m_callback->on_notification(rpcmsg);
        });
    }

} } }

#ifdef _WIN32
static string ConvertErrorCodeToString(DWORD ErrorCode)
{
    HLOCAL LocalAddress = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, ErrorCode, 0, (char*)&LocalAddress, 0, NULL);

    string ret((char*)LocalAddress);
    LocalFree(LocalAddress);
    return ret;
}
#endif

namespace tquant { namespace api {

    static unordered_map<string, string> g_params;

    static vector<T_create_trade_api> g_tapi_factories;

    void set_params(const string& key, const string& value)
    {
        g_params[key] = value;
    }

    DataApi* creatae_embed_dapi(const char* addr)
    {
        const char* p = strstr(addr, "://");
        if (!p) return nullptr;
        string module_name = string("tqapi_") + string(addr, p - addr);

#ifdef _WIN32
        string dirs = g_params["plugin_path"];
        vector<string> ss;
        split(dirs, ";", &ss);
        ss.push_back("");
        HMODULE hModule = nullptr;
        for (auto& path : ss) {
            string dll_path = (!path.empty() ? (string(path) + "\\") : "") + module_name + ".dll";
             hModule = LoadLibraryA(dll_path.c_str());
             if (hModule) break;
        }
        if (!hModule)
            throw std::runtime_error(ConvertErrorCodeToString(GetLastError()));

        auto create_data_api = (T_create_data_api)GetProcAddress(hModule, "create_data_api");
        if (!create_data_api) {
            FreeModule(hModule);
            throw std::runtime_error(ConvertErrorCodeToString(GetLastError()));
            return nullptr;
        }

        DataApi* dapi = create_data_api(p+3);
        if (!dapi) {
            FreeModule(hModule);
            return nullptr;
        }
        // FIXME: How to free module?
        return dapi;
#else
        string dirs = g_params["plugin_path"];
        vector<string> ss;
        split(dirs, ":", &ss);
        ss.push_back("");
        void * h = nullptr;
        for (auto& path : ss) {
            string dll_path = (!path.empty() ? (string(path) + "/lib") : "lib") + module_name + ".so";
            cout << "dlopen " << dll_path << endl;
            h = dlopen(dll_path.c_str(), RTLD_LAZY);
            if (h) break;
        }
        if (!h)
            throw std::runtime_error("Can't load library " + module_name);

        auto create_data_api = (T_create_data_api)dlsym(h, "create_data_api");
        if (!create_data_api) {
            dlclose(h);
            throw std::runtime_error("No create_data_api in " + module_name);
            return nullptr;
        }

        DataApi* dapi = create_data_api(p+3);
        if (!dapi) {
            dlclose(h);
            return nullptr;
        }
        // FIXME: How to free module?
        return dapi;
#endif
    }

    TradeApi* creatae_embed_tapi(const char* addr)
    {
        const char* p = strstr(addr, "://");
        if (!p) return nullptr;
        string module_name = string("tqapi_") + string(addr, p - addr);

#ifdef _WIN32

        string dirs = g_params["plugin_path"];
        vector<string> ss;
        split(dirs, ";", &ss);
        ss.push_back("");
        HMODULE hModule = nullptr;
        for (auto& path : ss) {
            string dll_path = (!path.empty() ? (string(path) + "\\") : "") + module_name + ".dll";
            hModule = LoadLibraryA(dll_path.c_str());
            if (hModule) break;
        }
        if (!hModule)
            throw std::runtime_error(ConvertErrorCodeToString(GetLastError()));

        auto create_trade_api = (T_create_trade_api)GetProcAddress(hModule, "create_trade_api");
        if (!create_trade_api) {
            FreeModule(hModule);
            throw std::runtime_error(ConvertErrorCodeToString(GetLastError()));
            return nullptr;
        }

        TradeApi* tapi = create_trade_api(p + 3);
        if (!tapi) {
            FreeModule(hModule);
            return nullptr;
        }
        // FIXME: How to free module?
        return tapi;
#else
        string dirs = g_params["plugin_path"];
        vector<string> ss;
        split(dirs, ":", &ss);
        ss.push_back("");
        void * h = nullptr;
        for (auto& path : ss) {
            string dll_path = (!path.empty() ? (string(path) + "/lib") : "lib") + module_name + ".so";
            cout << "dlopen " << dll_path << endl;
            h = dlopen(dll_path.c_str(), RTLD_LAZY);
            if (h) break;
        }
        if (!h)
            throw std::runtime_error("Can't load library " + module_name);

        auto create_trade_api = (T_create_trade_api)dlsym(h, "create_trade_api");
        if (!create_trade_api) {
            dlclose(h);
            throw std::runtime_error("No create_trade_api in " + module_name);
            return nullptr;
        }

        TradeApi* tapi = create_trade_api(p + 3);
        if (!tapi) {
            dlclose(h);
            return nullptr;
        }
        // FIXME: How to free module?
        return tapi;
#endif
    }
    void init_socket()
    {
#ifdef _WIN32
        static bool inited = false;
        if (!inited) {
            inited = true;
            myutils::init_winsock2();
        }
#endif
    }

    DataApi*  create_data_api(const string& addr)
    {
        init_socket();

        // std::cout<<"create_data_api " << addr<<std::endl;

        if (strncmp(addr.c_str(), "tcp://", 6) == 0 ||
            strncmp(addr.c_str(), "ipc://", 6) == 0) 
        {            
            string url;
            unordered_map<string, string> properties;
            if (!myutils::parse_addr(addr, &url, &properties)){
                return nullptr;
            } 

            auto conn = new impl::MpRpc_Connection();
            auto dapi = new impl::MpRpcDataApiImpl();

            if (conn->connect(url) && dapi->init(conn, properties)) {
                return dapi;
            } else {
                delete dapi;
                delete conn;
                return nullptr;
            }
        }
        else {
            return creatae_embed_dapi(addr.c_str());
        }
    }

    TradeApi* create_trade_api(const string& addr)
    {
        init_socket();

        if (strncmp(addr.c_str(), "tcp://", 6) == 0 ||
            strncmp(addr.c_str(), "ipc://", 6) == 0) {
            string url;
            unordered_map<string, string> properties;
            if (!myutils::parse_addr(addr, &url, &properties)) return nullptr;

            auto conn = new impl::MpRpc_Connection();
            auto tapi = new impl::MpRpcTradeApiImpl();

            if (conn->connect(url) && tapi->init(conn, properties)) {
                return tapi;
            }
            else {
                delete tapi;
                delete conn;
                return nullptr;
            }
        }
        else {
            for (auto& f : g_tapi_factories) {
                auto tapi = f(addr.c_str());
                if (tapi)
                    return tapi;
            }

            return creatae_embed_tapi(addr.c_str());
        }
    }


    void register_trade_api_factory(T_create_trade_api factory)
    {
        g_tapi_factories.push_back(factory);
    }
} }
