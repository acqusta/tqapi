#include "com_acqusta_tquant_api_impl_TQuantApiJni.h"
#include "tquant_api.h"
#include "tqapi_jni.h"

using namespace std;
using namespace tquant::api;

void throwJavaException(JNIEnv* env, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[2000];
    vsnprintf(buf, 20000, fmt, args);
    va_end(args);
    env->ThrowNew(env->FindClass("java/lang/Exception"), buf);
}

string get_string(JNIEnv* env, jstring str)
{
    if (!str) return "";
    const char* s = env->GetStringUTFChars(str, nullptr);
    if (!s)
        return string();
    string ret(s);
    env->ReleaseStringUTFChars(str, s);
    return ret;
}

/*
 * Class:     com_acqusta_tquant_api_impl_TQuantApiJni
 * Method:    create
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_acqusta_tquant_api_impl_TQuantApiJni_create
  (JNIEnv * env, jclass, jstring addr)
{
    try {
        const char* cs = env->GetStringUTFChars(addr, 0);
        if (!cs)
            return 0;
        string s(cs);
        env->ReleaseStringUTFChars(addr, cs);
        auto api = TQuantApi::create(s.c_str());
        if (api) {
            auto wrap = new TQuantApiWrap();
            wrap->api = api;
            wrap->init(env);
            return reinterpret_cast<jlong>(wrap);
        }
        else {
            return 0;
        }
    }
    catch (const exception& e) {
        throwJavaException(env, "%s", e.what());
        return 0;
    }
}

/*
 * Class:     com_acqusta_tquant_api_impl_TQuantApiJni
 * Method:    destroy
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_api_impl_TQuantApiJni_destroy
  (JNIEnv *env, jclass cls, jlong handle)
{
    try {
        auto api = reinterpret_cast<TQuantApiWrap*>(handle);
        if (api) api->destroy(env);
    }
    catch (const exception& e) {
        throwJavaException(env, "%s", e.what());
    }
}

/*
 * Class:     com_acqusta_tquant_api_impl_TQuantApiJni
 * Method:    getTradeApi
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_acqusta_tquant_api_impl_TQuantApiJni_getTradeApi
    (JNIEnv *env, jclass, jlong handle)
{
    return handle;
}

/*
 * Class:     com_acqusta_tquant_api_impl_TQuantApiJni
 * Method:    getDataApi
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_acqusta_tquant_api_impl_TQuantApiJni_getDataApi
    (JNIEnv *env, jclass, jlong handle, jstring source)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(handle);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    // Java code should only call getDataApi once for a same source!
    try {
        std::string s_source = get_string(env, source);
        auto dapi = wrap->api->data_api(s_source.c_str());
        if (dapi)
            return reinterpret_cast<jlong>(new DataApiWrap(wrap, dapi));
        else
            throwJavaException(env, "can't create DataApi");
    }
    catch (const std::exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return 0;
    }
}

bool TQuantApiWrap::init(JNIEnv* env)
{
    do {
        this->help_cls = (jclass)env->FindClass("Lcom/acqusta/tquant/api/impl/JniHelper;");
        if (!this->help_cls) {
            throwJavaException(env, "No class JniHelper");
            break;
        }
        this->help_cls = (jclass)env->NewGlobalRef(this->help_cls);

        this->createMarketQuote = env->GetStaticMethodID(help_cls, "createMarketQuote",
            "(Ljava/lang/String;"
            "III"
            "DDDDDDDD"
            "JD"
            "DDDDDDDDDDDDDDDDDDDD"
            "JJJJJJJJJJJJJJJJJJJJ"
            "DDJJ"
            ")Lcom/acqusta/tquant/api/DataApi$MarketQuote;");
        if (!createMarketQuote) {
            throwJavaException(env, "No createMarketQuote in JniHelper");
            break;
        }
        createBar = env->GetStaticMethodID(help_cls, "createBar",
            "(Ljava/lang/String;"
            "III"
            "DDDD"
            "JDJ"
            ")Lcom/acqusta/tquant/api/DataApi$Bar;");
        if (!createBar) {
            throwJavaException(env, "No createBar in JniHelper");
            break;
        }

        createDailyBar = env->GetStaticMethodID(help_cls, "createDailyBar",
            "(Ljava/lang/String;"
            "I"
            "DDDD"
            "JDJ"
            "DDD"
            ")Lcom/acqusta/tquant/api/DataApi$DailyBar;");
        if (!createDailyBar) {
            throwJavaException(env, "No createDailyBar in JniHelper");
            break;
        }

        createMarketQuote = env->GetStaticMethodID(help_cls, "createMarketQuote",
            "(Ljava/lang/String;"
            "III"
            "DDDDDDDD"
            "JD"
            "DDDDDDDDDDDDDDDDDDDD"
            "JJJJJJJJJJJJJJJJJJJJ"
            "DDJJ"
            ")Lcom/acqusta/tquant/api/DataApi$MarketQuote;");
        if (!createMarketQuote) {
            throwJavaException(env, "No createMarketQuote in JniHelper");
            break;
        }

        createAccountInfo = env->GetStaticMethodID(help_cls, "createAccountInfo",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
            "Ljava/lang/String;Ljava/lang/String;)Lcom/acqusta/tquant/api/TradeApi$AccountInfo;");
        if (!createAccountInfo) {
            throwJavaException(env, "No createAccountInfo in JniHelper");
            break;
        }

        createBalance = env->GetStaticMethodID(help_cls, "createBalance",
            "(Ljava/lang/String;Ljava/lang/String;DDDDD)Lcom/acqusta/tquant/api/TradeApi$Balance;");

        if (!createBalance) {
            throwJavaException(env, "No createBalance in JniHelper");
            break;
        }

        createOrder = env->GetStaticMethodID(help_cls, "createOrder",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
            "Ljava/lang/String;Ljava/lang/String;"
            "IIDJDJ"
            "Ljava/lang/String;Ljava/lang/String;"
            "I)Lcom/acqusta/tquant/api/TradeApi$Order;");

        if (!createOrder) {
            throwJavaException(env, "No createOrder in JniHelper");
            break;
        }

        createTrade = env->GetStaticMethodID(help_cls, "createTrade",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
            "Ljava/lang/String;Ljava/lang/String;"
            "Ljava/lang/String;"
            "DJII"
            ")Lcom/acqusta/tquant/api/TradeApi$Trade;");

        if (!createOrder) {
            throwJavaException(env, "No createOrder in JniHelper");
            break;
        }

        createPosition = env->GetStaticMethodID(help_cls, "createPosition",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
            "JJJJJ"
            "Ljava/lang/String;"
            "DDDDDDD"
            ")Lcom/acqusta/tquant/api/TradeApi$Position;");

        if (!createPosition) {
            throwJavaException(env, "No createPosition in JniHelper");
            break;
        }

        createOrderID = env->GetStaticMethodID(help_cls, "createOrderID",
            "(Ljava/lang/String;I"
            ")Lcom/acqusta/tquant/api/TradeApi$OrderID;");

        if (!createPosition) {
            throwJavaException(env, "No createOrderID in JniHelper");
            break;
        }

         env->GetJavaVM(&this->jvm);

         m_dapi_loop.msg_loop().PostTask([this]() {
             int r = jvm->GetEnv((void**)&dapi_jenv, JNI_VERSION_1_6);
             if (r == JNI_EDETACHED)
                 jvm->AttachCurrentThreadAsDaemon((void**)&dapi_jenv, nullptr);
         });

         m_tapi_loop.msg_loop().PostTask([this]() {
             int r = jvm->GetEnv((void**)&tapi_jenv, JNI_VERSION_1_6);
             if (r == JNI_EDETACHED)
                 jvm->AttachCurrentThreadAsDaemon((void**)&tapi_jenv, nullptr);
         });

         api->trade_api()->set_callback(this);
        return true;
    } while (false);

    RELEASE_JOBJECT(help_cls);
    return false;
}

void TQuantApiWrap::destroy(JNIEnv* env)
{
    if (this->api) {
        delete this->api;
        this->api = nullptr;
    }

    m_dapi_loop.msg_loop().PostTask([this]() {
        jvm->DetachCurrentThread();
    });

    m_tapi_loop.msg_loop().PostTask([this]() {
        jvm->DetachCurrentThread();
    });

    RELEASE_JOBJECT(help_cls);
    RELEASE_JOBJECT(tapi_callback);
}

