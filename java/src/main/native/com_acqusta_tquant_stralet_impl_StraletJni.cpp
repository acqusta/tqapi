#include "com_acqusta_tquant_stralet_impl_StraletJni.h"
#include "tquant_api.h"
#include "stralet.h"
#include "bt/backtest.h"
#include "rt/realtime.h"
#include "tqapi_jni.h"

using namespace tquant::stralet;

class StraletWrap : public Stralet, public JniObjectCreator {
public:
    JNIEnv* jenv     = nullptr;
    jobject m_stralet = nullptr;

    jmethodID m_setContext;
    jmethodID m_onInit;
    jmethodID m_onFini;
    jmethodID m_onQuote;
    jmethodID m_onBar;
    jmethodID m_onOrder;
    jmethodID m_onTrade;
    jmethodID m_onAccountStatus;
    jmethodID m_onEvent;
    jmethodID m_onTimer;


    StraletWrap(JNIEnv* env)
        : JniObjectCreator(env)
        , jenv(env)
    {
    }

    ~StraletWrap()
    {
        destroy();
    }

    bool init(jobject stralet);

    void destroy() {
        if (m_stralet) {
            jenv->DeleteGlobalRef(m_stralet);
            m_stralet = nullptr;
        }
    }

    virtual void on_init() override
    {
        jenv->CallVoidMethod(m_stralet, m_setContext, (jlong)ctx());
        if (jenv->ExceptionCheck())
            throw exception("catch exception in JVM");

        jenv->CallVoidMethod(m_stralet, m_onInit);

        if (jenv->ExceptionCheck())
            throw exception("catch exception in JVM");
    }

    virtual void on_fini() override
    {
        jenv->CallVoidMethod(m_stralet, m_onFini);

        if (jenv->ExceptionCheck())
            throw exception("catch exception in JVM");

        destroy();
    }

    virtual void on_quote(shared_ptr<const MarketQuote> quote) override
    {
        auto q = convert_quote(jenv, help_cls, createMarketQuote, quote.get());
        jenv->CallVoidMethod(m_stralet, m_onQuote, q);
        jenv->DeleteLocalRef(q);
    }

    virtual void on_bar(const string& cycle, shared_ptr<const Bar> bar) override
    {
        auto b = convert_bar(jenv, help_cls, createBar, bar.get());
        jenv->CallVoidMethod(m_stralet, m_onBar, jenv->NewStringUTF(cycle.c_str()), b);
        jenv->DeleteLocalRef(b);
    }

    virtual void on_order(shared_ptr<const Order> order) override
    {
        auto o = convert_order(jenv, help_cls, createOrder, order.get());
        jenv->CallVoidMethod(m_stralet, m_onOrder, o);
        jenv->DeleteLocalRef(o);
    }

    virtual void on_trade(shared_ptr<const Trade> trade) override
    {
        auto o = convert_trade(jenv, help_cls, createTrade, trade.get());
        jenv->CallVoidMethod(m_stralet, m_onTrade, o);
        jenv->DeleteLocalRef(o);
    }

    virtual void on_timer(int64_t id, void* data) override
    {
        jenv->CallVoidMethod(m_stralet, m_onTimer, (jlong)id, (jlong)data);
    }

    virtual void on_event(const string& name, void* data) override
    {
        LocalRef s_name(jenv, jenv->NewStringUTF(name.c_str()));
        jenv->CallVoidMethod(m_stralet, m_onEvent, s_name.m_obj, (jlong)data);
    }

    virtual void on_account_status(shared_ptr<const AccountInfo> account) override
    {
        auto o = convert_account_info(jenv, help_cls, createAccountInfo, account.get());
        jenv->CallVoidMethod(m_stralet, m_onTrade, o);
        jenv->DeleteLocalRef(o);
    }

};

bool StraletWrap::init(jobject stralet)
{
    jclass stralet_class = jenv->GetObjectClass(stralet);

    m_setContext = jenv->GetMethodID(stralet_class, "setContext", "(J)V");
    if (!m_setContext) {
        throwJavaException(jenv, "No setContext in Stralet");
        return false;
    }


    m_onInit = jenv->GetMethodID(stralet_class, "onInit", "()V");
    if (!m_onInit) {
        throwJavaException(jenv, "No onInit in Stralet");
        return false;
    }

    m_onFini = jenv->GetMethodID(stralet_class, "onFini", "()V");
    if (!m_onFini) {
        throwJavaException(jenv, "No onFini in Stralet");
        return false;
    }

    m_onQuote = jenv->GetMethodID(stralet_class, "onQuote",
        "(Lcom/acqusta/tquant/api/DataApi$MarketQuote;)V");
    if (!m_onQuote) {
        throwJavaException(jenv, "No onQuote in Stralet");
        return false;
    }

    m_onBar = jenv->GetMethodID(stralet_class, "onBar",
        "(Ljava/lang/String;Lcom/acqusta/tquant/api/DataApi$Bar;)V");

    if (!m_onBar) {
        throwJavaException(jenv, "No onBar in Stralet");
        return false;
    }

    m_onOrder = jenv->GetMethodID(stralet_class, "onOrder",
        "(Lcom/acqusta/tquant/api/TradeApi$Order;)V");
    if (!m_onOrder) {
        throwJavaException(jenv, "No onOrder in Stralet");
        return false;
    }

    m_onTrade = jenv->GetMethodID(stralet_class, "onTrade",
        "(Lcom/acqusta/tquant/api/TradeApi$Trade;)V");

    if (!m_onTrade) {
        throwJavaException(jenv, "No onTrade in Stralet");
        return false;
    }

    m_onAccountStatus = jenv->GetMethodID(stralet_class, "onAccountStatus",
        "(Lcom/acqusta/tquant/api/TradeApi$AccountInfo;)V");

    if (!m_onAccountStatus) {
        throwJavaException(jenv, "No onAccountStatus in TataApi$Callback");
        return false;
    }

    m_onTimer = jenv->GetMethodID(stralet_class, "onTimer", "(JJ)V");
    m_onEvent = jenv->GetMethodID(stralet_class, "onEvent", "(Ljava/lang/String;J)V");

    m_stralet = jenv->NewGlobalRef(stralet);

    return true;
}


class StraletCreator {// : public JniHelper {
public:
    jobject   m_creator = nullptr;
    jmethodID m_createStralet = nullptr;
    JNIEnv*   jenv;
    StraletCreator(JNIEnv* env)
        : jenv(env)
    {
    }

    ~StraletCreator()
    {
        destroy();
    }

    bool init(jobject java_creator);
    void destroy();

    Stralet* createStralet();
};

bool StraletCreator::init(jobject java_creator)
{
    jclass creator_class = jenv->GetObjectClass(java_creator);
    m_createStralet = jenv->GetMethodID(creator_class, "createStralet", "()Lcom/acqusta/tquant/stralet/Stralet;");
    if (!m_createStralet){
        throwJavaException(jenv, "No createStralet in StraletCreator");
        return false;
    }

    m_creator = jenv->NewGlobalRef(java_creator);
    
    return true;
}

void StraletCreator::destroy()
{
    if (m_creator) {
        jenv->DeleteGlobalRef(m_creator);
        m_creator = nullptr;
    }
}

Stralet* StraletCreator::createStralet()
{
    auto java_stralet = jenv->CallObjectMethod(m_creator, m_createStralet);
    auto stralet = new StraletWrap(jenv);
    if (!stralet->init(java_stralet)) {
        delete stralet;
        throw exception("Exception in createStralet");
    }
    return stralet;
}

JNIEXPORT jlong JNICALL Java_com_acqusta_tquant_stralet_impl_StraletJni_create
  (JNIEnv *env, jclass cls, jobject java_stralet)
{
    auto wrap = new StraletWrap(env);
    if (wrap->init(java_stralet)) {
        return reinterpret_cast<jlong>(wrap);
    } else {
        delete wrap;
        return 0;
    }
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletJni
 * Method:    destroy
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_stralet_impl_StraletJni_destroy
  (JNIEnv * env, jclass cls, jlong h)
{
    if (!h) return;

    StraletWrap* wrap = reinterpret_cast<StraletWrap*>(h);
    if (!h) {
        throwJavaException(env, "Wrong Stralet handle");
        return;
    }

    delete wrap;
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletJni
 * Method:    runBacktest
 * Signature: (Ljava/lang/String;Lcom/acqusta/tquant/stralet/StraletCreator;)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_stralet_impl_StraletJni_runBacktest
  (JNIEnv * env, jclass cls, jstring cfg, jobject stralet_creator)
{
    string text = get_string(env, cfg);
    auto creator = new StraletCreator(env);
    if (!creator->init(stralet_creator)) {
        delete creator;
        return;
    }

    try {
        backtest::run(text.c_str(), [creator]() { return creator->createStralet(); });
    }
    catch(...)
    { }

    delete creator;
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletJni
 * Method:    runRealTime
 * Signature: (Ljava/lang/String;Lcom/acqusta/tquant/stralet/StraletCreator;)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_stralet_impl_StraletJni_runRealTime
(JNIEnv * env, jclass cls, jstring cfg, jobject stralet_creator)
{
    string text = get_string(env, cfg);
    auto creator = new StraletCreator(env);
    if (!creator->init(stralet_creator)) {
        delete creator;
        return;
    }

    try {
        realtime::run(text.c_str(), [creator]() { return creator->createStralet(); });
    }
    catch (...)
    {
    }

    delete creator;
}
