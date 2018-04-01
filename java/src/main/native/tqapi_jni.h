#ifndef _TQAPI_JNI_H
#define _TQAPI_JNI_H

#include <string>
#include <mutex>
#include "myutils/loop/MsgRunLoop.h"

#include "tquant_api.h"

using namespace std;
using namespace tquant::api;

#define RELEASE_JOBJECT(_obj_)          \
    if (_obj_) {                        \
        env->DeleteGlobalRef(_obj_);    \
        _obj_ = nullptr;                \
    }


class TQuantApiWrap :
    public tquant::api::TradeApi_Callback {

public:
    virtual void on_order_status(shared_ptr<Order> order) override;
    virtual void on_order_trade(shared_ptr<Trade> trade) override;
    virtual void on_account_status(shared_ptr<AccountInfo> account) override;


    tquant::api::TQuantApi* api;
    JavaVM* jvm;

    jclass help_cls;
    jmethodID createMarketQuote;
    jmethodID createBar;
    jmethodID createDailyBar;
    jmethodID createAccountInfo;
    jmethodID createBalance;
    jmethodID createOrder;
    jmethodID createTrade;
    jmethodID createPosition;
    jmethodID createOrderID;

    jmethodID dapi_onMarketQuote;
    jmethodID dapi_onBar;

    jmethodID tapi_onOrderStatus;
    jmethodID tapi_onOrderTrade;
    jmethodID tapi_onAccountStatus;

    jobject tapi_callback;

    JNIEnv* dapi_jenv;
    JNIEnv* tapi_jenv;

    loop::MsgLoopRun  m_dapi_loop;
    loop::MsgLoopRun  m_tapi_loop;

    TQuantApiWrap()
        : jvm(nullptr)
        , help_cls(nullptr) 
        , api(nullptr)
        , tapi_callback(nullptr)
        , dapi_jenv(nullptr)
        , tapi_jenv(nullptr)
    {}

    bool init(JNIEnv* env);
    void destroy(JNIEnv* env);
private:
    ~TQuantApiWrap() {}
};

class DataApiWrap : public tquant::api::DataApi_Callback {
public:
    DataApiWrap(TQuantApiWrap*tqapi, DataApi* dapi)
        : m_tqapi(tqapi)
        , m_dapi(dapi)
        , m_dapi_callback(nullptr)
    {        
        m_dapi->set_callback(this);
    }

    ~DataApiWrap() {

    }

    void destroy(JNIEnv* env) {
        RELEASE_JOBJECT(m_dapi_callback);
    }

    virtual void on_market_quote(shared_ptr<const MarketQuote> quote) override;
    virtual void on_bar(const string& cycle, shared_ptr<const Bar> bar) override;

    TQuantApiWrap* m_tqapi;
    DataApi*       m_dapi;
    jobject        m_dapi_callback;
};

class LocalRef {
public:
    LocalRef(JNIEnv* env, jobject obj) : m_env(env), m_obj(obj)
    {}

    ~LocalRef() {
        m_env->DeleteLocalRef(m_obj);
    }

    JNIEnv* m_env;
    jobject m_obj;
};

void throwJavaException(JNIEnv* env, const char* fmt, ...);

std::string get_string(JNIEnv* env, jstring str);


#endif
