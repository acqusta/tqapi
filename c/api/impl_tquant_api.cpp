#include <sstream>
#include <unordered_set>
#include "tquant_api.h"
#include "myutils/stringutils.h"
#include "myutils/mprpc.h"
#include "myutils/socket_connection.h"
#include "myutils/ipc_connection.h"
#include "myutils/socketutils.h"
#include "impl_tquant_api.h"
#include "impl_data_api.h"
#include "impl_trade_api.h"

namespace tquant { namespace api { namespace impl {

    using namespace ::mprpc;
    using namespace ::tquant::api;
    using namespace ::myutils;

    class TQuantApiImpl : public TQuantApi, public MpRpcClient_Callback {
        friend DataApiImpl;
        friend TradeApiImpl;
    public:
        TQuantApiImpl(const string& params) {
#if 0
            const char* p = strchr(params, '?');
            string addr;
            string source;
            if (p) {
                addr.assign(params, p - params);
                p += 1;
                vector<string> ss;
                split(p, "&", &ss);
                for (auto& s : ss) {
                    vector<string> tmp;
                    split(s, "=", &tmp);
                    if (tmp[0] == "source") {
                        source = tmp[1];
                    }
                }
            }
            else {
                addr = params;
            }
#else
            string addr = params;
#endif

            if (strncmp(addr.c_str(), "tcp://", 6) == 0) {
                auto conn = make_shared<SocketConnection>();
                m_client = new MpRpcClient(conn);
                m_client->connect(addr, this);
            }
            else if (strncmp(addr.c_str(), "ipc://", 6) == 0) {
                auto conn = make_shared<IpcConnection>();
                m_client = new MpRpcClient(conn);
                m_client->connect(addr, this);
            }
            else {
                throw std::runtime_error("unknown addr");
            }

            m_tapi = new TradeApiImpl(this->m_client);
        }

        virtual ~TQuantApiImpl() override {
            m_msgloop.close_loop();
            for (auto e : m_dapi_map) delete e.second;
            delete m_tapi;
            delete m_client;
        }

        virtual TradeApi* trade_api() override { return m_tapi; }

        virtual DataApi*  data_api(const string& source) override {
            auto it = m_dapi_map.find(source);
            if (it != m_dapi_map.end())
                return it->second;
            auto dapi = new DataApiImpl(this->m_client, source);
            this->m_dapi_map[source] = dapi;
            return dapi;
        }

        virtual void on_connected() override {}

        virtual void on_disconnected() override {}

        virtual void on_notification(shared_ptr<MpRpcMessage> rpcmsg) override
        {
            m_msgloop.msg_loop().PostTask([this, rpcmsg] {
                if (strncmp(rpcmsg->method.c_str(), "dapi.", 5) == 0) {
                    for (auto e : m_dapi_map)
                        e.second->on_notification(rpcmsg);
                }
                else if (strncmp(rpcmsg->method.c_str(), "tapi.", 5) == 0) {
                    m_tapi->on_notification(rpcmsg);
                }
                else {
                    for (auto e : m_dapi_map)
                        e.second->on_notification(rpcmsg);
                    m_tapi->on_notification(rpcmsg);
                }
            });
        }
    private:
        MpRpcClient*                            m_client;
        unordered_map<string, DataApiImpl*>     m_dapi_map;
        TradeApiImpl*                           m_tapi;
        loop::MsgLoopRun                        m_msgloop;
    };

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

    typedef TQuantApi* (*T_create_tqapi)(const char* str_params);

    TQuantApi* creatae_embedapi(const string& addr)
    {
#ifdef _WIN32
        vector<string> ss;
        split(addr, "?", &ss);
        const char* p = ss[0].c_str() + 8;
        string module_name = string("embed_") + string(p);
        HMODULE hModule = LoadLibraryA(module_name.c_str());
        if (!hModule)
            throw std::runtime_error(ConvertErrorCodeToString(GetLastError()));
        auto create_tqapi = (T_create_tqapi)GetProcAddress(hModule, "create_tqapi");
        if (!create_tqapi) {
            FreeModule(hModule);
            throw std::runtime_error(ConvertErrorCodeToString(GetLastError()));
            return nullptr;
        }
        TQuantApi* tqapi = create_tqapi(addr.c_str());
        if (!tqapi) {
            FreeModule(hModule);
            return nullptr;
        }
        // FIXME: How to free module?
        return tqapi;
#else
        throw std::runtime_erro("to be implemented");
#endif
    }

    TQuantApi* TQuantApi::create(const string& addr)
    {
#ifdef _WIN32
        static bool inited = false;
        if (!inited) {
            inited = true;
            myutils::init_winsock2();
        }
#endif
        if (strncmp(addr.c_str(), "embed://", 8) == 0)
            return creatae_embedapi(addr);
        else
            return new impl::TQuantApiImpl(addr);
    }

} }
