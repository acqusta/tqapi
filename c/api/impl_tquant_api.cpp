#include <sstream>
#include <unordered_set>
#include "tquant_api.h"
#include "myutils/stringutils.h"
#include "myutils/mprpc.h"
#include "myutils/socket_connection.h"
#include "myutils/ipc_connection.h"
#include "myutils/socketutils.h"
#include "myutils/misc.h"
#include "impl_tquant_api.h"
#include "impl_data_api.h"
#include "impl_trade_api.h"
#include "tquant_api.h"

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
        m_msgloop.msg_loop().PostTask([this, rpcmsg] {
            if (m_callback) m_callback->on_notification(rpcmsg);
        });
    }

} } }

#ifdef _WIN32
static
string ConvertErrorCodeToString(DWORD ErrorCode)
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

    typedef DataApi*  (*T_create_data_api)(const char* str_params);
    typedef TradeApi* (*T_create_trade_api)(const char* str_params);

    // addr = "embed://tkapi/file://d:/tquant/md"

    static unordered_map<string, string> g_params;

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
        throw std::runtime_erro("to be implemented");
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

        //if (strncmp(addr.c_str(), "embed://", 8)) {
        if (strncmp(addr.c_str(), "tcp://", 6) == 0 ||
            strncmp(addr.c_str(), "ipc://", 6) == 0) 
        {
            string url;
            unordered_map<string, string> properties;
            if (!myutils:: parse_addr(addr, &url, &properties)) return nullptr;

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

        if (strncmp(addr.c_str(), "embed://", 8)) {
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
            return nullptr;
        }

    }


} }
