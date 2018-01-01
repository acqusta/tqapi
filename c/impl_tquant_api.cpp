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

            m_dapi = new DataApiImpl(this->m_client);
            m_tapi = new TradeApiImpl(this->m_client);
        }

        virtual ~TQuantApiImpl() override {
            delete m_dapi;
            delete m_tapi;
            delete m_client;
        }

        virtual TradeApi* trade_api() override { return m_tapi; }

        virtual DataApi*  data_api() override  { return m_dapi; }

        virtual void on_connected() override {}

        virtual void on_disconnected() override {}

        virtual void on_notification(shared_ptr<MpRpcMessage> rpcmsg) override
        {
            if (strncmp(rpcmsg->method.c_str(), "dapi.", 5) == 0) {
                m_dapi->on_notification(rpcmsg);
            }
            else if (strncmp(rpcmsg->method.c_str(), "tapi.", 5) == 0) {
                m_tapi->on_notification(rpcmsg);
            }
            else  {
                m_dapi->on_notification(rpcmsg);
                m_tapi->on_notification(rpcmsg);
            }
        }
        virtual void on_call_result(int callid, shared_ptr<MpRpcMessage> cr) override
        {}
    private:
        MpRpcClient*  m_client;
        DataApiImpl*    m_dapi;
        TradeApiImpl*   m_tapi;
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
