#ifndef _TRADE_API_IMPL_H
#define _TRADE_API_IMPL_H

#include <stdint.h>
#include <sstream>
#include <string>

#include "impl_tquant_api.h"

namespace tquant { namespace api { namespace impl {

    using namespace std;
    using namespace mprpc;

    class MpRpc_Connection : public MpRpcClient_Callback {
    public:
        MpRpc_Connection();
        ~MpRpc_Connection();

        bool connect(const string& addr);

        void set_callback(MpRpcClient_Callback*  cb) { this->m_callback = cb; }

        virtual void on_connected() override;
        virtual void on_disconnected() override;
        virtual void on_notification(shared_ptr<MpRpcMessage> rpcmsg) override;

        MpRpcClient_Callback* m_callback;
        MpRpcClient*          m_client;
        loop::MsgLoopRun      m_msgloop;

    };

} } }

#endif
