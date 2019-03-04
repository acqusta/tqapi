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


class JniObjectCreator {
    JNIEnv*   env;
public:
    jclass    help_cls;
    jmethodID createMarketQuote;
    jmethodID createBar;
    jmethodID createDailyBar;
    jmethodID createAccountInfo;
    jmethodID createBalance;
    jmethodID createOrder;
    jmethodID createTrade;
    jmethodID createPosition;
    jmethodID createOrderID;

    JniObjectCreator(JNIEnv* jenv);
    ~JniObjectCreator();
};

class JniCallbackHelper : public loop::MsgLoopRun {
public:
    JavaVM*   jvm;
    JNIEnv*   jenv;
    jmethodID dapi_onMarketQuote;
    jmethodID dapi_onBar;

    jmethodID tapi_onOrderStatus;
    jmethodID tapi_onOrderTrade;
    jmethodID tapi_onAccountStatus;

    JniCallbackHelper(JNIEnv* env);

    ~JniCallbackHelper();

    void destroy(JNIEnv* env);
};

class DataApiWrap : public tquant::api::DataApi_Callback {
public:
    DataApiWrap(DataApi* dapi, JNIEnv* env, bool has_callback=true)
        : m_dapi(dapi)
        , m_dapi_callback(nullptr)
    {
        m_obj_creator = new JniObjectCreator(env);
        if (has_callback) {
            m_callback = new JniCallbackHelper(env);
            m_dapi->set_callback(this);
        }
        else {
            m_callback = nullptr;
        }
    }

    ~DataApiWrap() {

    }

    void destroy(JNIEnv* env) {
        if (m_callback) {
            delete m_callback;
            m_callback = nullptr;
        }

        if (m_obj_creator) {
            delete m_obj_creator;
            m_obj_creator = nullptr;
        }

        RELEASE_JOBJECT(m_dapi_callback);
    }

    virtual void on_market_quote(shared_ptr<const MarketQuote> quote) override;
    virtual void on_bar(const string& cycle, shared_ptr<const Bar> bar) override;

    DataApi*           m_dapi;
    jobject            m_dapi_callback;
    JniObjectCreator*  m_obj_creator;
    JniCallbackHelper* m_callback;
};

class TradeApiWrap :public tquant::api::TradeApi_Callback {
public:
    TradeApiWrap(TradeApi* tapi, JNIEnv* env, bool has_callback=true)
        : m_tapi(tapi)
        , m_tapi_callback(nullptr)
    {
        m_obj_creator = new JniObjectCreator(env);
        if (has_callback) {
            m_callback = new JniCallbackHelper(env);
            m_tapi->set_callback(this);
        }
        else {
            m_callback = nullptr;
        }
    }

    ~TradeApiWrap() {

    }

    void destroy(JNIEnv* env) {
        if (m_callback) {
            delete m_callback;
            m_callback = nullptr;
        }

        if (m_obj_creator) {
            delete m_obj_creator;
            m_obj_creator = nullptr;
        }

        RELEASE_JOBJECT(m_tapi_callback);
    }

    virtual void on_order_status(shared_ptr<Order> order) override;
    virtual void on_order_trade(shared_ptr<Trade> trade) override;
    virtual void on_account_status(shared_ptr<AccountInfo> account) override;

    TradeApi*          m_tapi;
    jobject            m_tapi_callback;
    JniObjectCreator*  m_obj_creator;
    JniCallbackHelper* m_callback;

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

jobject convert_order        (JNIEnv* env, jclass help_cls, jmethodID createOrder, const tquant::api::Order* ord);
jobject convert_trade        (JNIEnv* env, jclass help_cls, jmethodID createTrade, const tquant::api::Trade* trd);
jobject convert_account_info (JNIEnv* env, jclass help_cls, jmethodID createAccountInfo, const tquant::api::AccountInfo* act);
jobject convert_quote        (JNIEnv* env, jclass help_cls, jmethodID createMarketQuote, const tquant::api::RawMarketQuote* q);
jobject convert_bar          (JNIEnv* env, jclass help_cls, jmethodID createBar, const tquant::api::RawBar* bar);

#endif
