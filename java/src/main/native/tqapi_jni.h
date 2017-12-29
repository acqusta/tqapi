#ifndef _TQAPI_JNI_H
#define _TQAPI_JNI_H

#include <string>
#include <mutex>
#include "loop/MsgRunLoop.h"

#include "tquant_api.h"

using namespace std;
using namespace tquant::api;

class TQuantApiWrap :
    public tquant::api::DataApi_Callback ,
    public tquant::api::TradeApi_Callback {

public:
    virtual void on_market_quote(shared_ptr<MarketQuote> quote) override;
    virtual void on_bar(const char* cycle, shared_ptr<Bar> bar) override;
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

    jobject dapi_callback;
    jobject tapi_callback;

    JNIEnv* dapi_jenv;
    JNIEnv* tapi_jenv;

    loop::MsgLoopRun  m_dapi_loop;
    loop::MsgLoopRun  m_tapi_loop;

    TQuantApiWrap()
        : jvm(nullptr)
        , help_cls(nullptr) 
        , api(nullptr)
        , dapi_callback(nullptr)
        , tapi_callback(nullptr)
        , dapi_jenv(nullptr)
        , tapi_jenv(nullptr)
    {}

    bool init(JNIEnv* env);
    void destroy(JNIEnv* env);
private:
    ~TQuantApiWrap() {}
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