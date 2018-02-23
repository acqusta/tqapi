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
        TQuantApiImpl(const char* params) {
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

        virtual DataApi*  data_api(const char* source) override {
            string str = source ? source : "";
            auto it = m_dapi_map.find(str);
            if (it != m_dapi_map.end())
                return it->second;
            auto dapi = new DataApiImpl(this->m_client, str.c_str());
            this->m_dapi_map[str] = dapi;
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

namespace tquant { namespace api {

    TQuantApi* TQuantApi::create(const char* addr)
    {
#ifdef _WIN32
        static bool inited = false;
        if (!inited) {
            inited = true;
            myutils::init_winsock2();
        }
#endif
        return new impl::TQuantApiImpl(addr);
    }

} }
