#include <jni.h>
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

JniHelper::JniHelper(JNIEnv* env)
{
    this->help_cls = (jclass)env->FindClass("Lcom/acqusta/tquant/api/impl/JniHelper;");
    if (!this->help_cls) {
        throwJavaException(env, "No class JniHelper");
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
    if (!createMarketQuote) 
        throwJavaException(env, "No createMarketQuote in JniHelper");

    createBar = env->GetStaticMethodID(help_cls, "createBar",
        "(Ljava/lang/String;"
        "III"
        "DDDD"
        "JDJ"
        ")Lcom/acqusta/tquant/api/DataApi$Bar;");
    if (!createBar)
        throwJavaException(env, "No createBar in JniHelper");

    createDailyBar = env->GetStaticMethodID(help_cls, "createDailyBar",
        "(Ljava/lang/String;"
        "I"
        "DDDD"
        "JDJ"
        "DDD"
        ")Lcom/acqusta/tquant/api/DataApi$DailyBar;");

    if (!createDailyBar)
        throwJavaException(env, "No createDailyBar in JniHelper");

    createMarketQuote = env->GetStaticMethodID(help_cls, "createMarketQuote",
        "(Ljava/lang/String;"
        "III"
        "DDDDDDDD"
        "JD"
        "DDDDDDDDDDDDDDDDDDDD"
        "JJJJJJJJJJJJJJJJJJJJ"
        "DDJJ"
        ")Lcom/acqusta/tquant/api/DataApi$MarketQuote;");

    if (!createMarketQuote)
        throwJavaException(env, "No createMarketQuote in JniHelper");

    createAccountInfo = env->GetStaticMethodID(help_cls, "createAccountInfo",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
        "Ljava/lang/String;Ljava/lang/String;)Lcom/acqusta/tquant/api/TradeApi$AccountInfo;");
    if (!createAccountInfo)
        throwJavaException(env, "No createAccountInfo in JniHelper");

    createBalance = env->GetStaticMethodID(help_cls, "createBalance",
        "(Ljava/lang/String;Ljava/lang/String;DDDDD)Lcom/acqusta/tquant/api/TradeApi$Balance;");

    if (!createBalance)
        throwJavaException(env, "No createBalance in JniHelper");

    createOrder = env->GetStaticMethodID(help_cls, "createOrder",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
        "Ljava/lang/String;Ljava/lang/String;"
        "IIDJDJ"
        "Ljava/lang/String;Ljava/lang/String;"
        "I)Lcom/acqusta/tquant/api/TradeApi$Order;");

    if (!createOrder)
        throwJavaException(env, "No createOrder in JniHelper");

    createTrade = env->GetStaticMethodID(help_cls, "createTrade",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
        "Ljava/lang/String;Ljava/lang/String;"
        "Ljava/lang/String;"
        "DJII"
        ")Lcom/acqusta/tquant/api/TradeApi$Trade;");

    if (!createOrder)
        throwJavaException(env, "No createOrder in JniHelper");

    createPosition = env->GetStaticMethodID(help_cls, "createPosition",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;"
        "JJJJJ"
        "Ljava/lang/String;"
        "DDDDDDD"
        ")Lcom/acqusta/tquant/api/TradeApi$Position;");

    if (!createPosition)
        throwJavaException(env, "No createPosition in JniHelper");

    createOrderID = env->GetStaticMethodID(help_cls, "createOrderID",
        "(Ljava/lang/String;I"
        ")Lcom/acqusta/tquant/api/TradeApi$OrderID;");

    if (!createPosition)
        throwJavaException(env, "No createOrderID in JniHelper");

    env->GetJavaVM(&this->jvm);

    msg_loop().post_task([this]() {
        int r = jvm->GetEnv((void**)&jenv, JNI_VERSION_1_6);
        if (r == JNI_EDETACHED)
            jvm->AttachCurrentThreadAsDaemon((void**)&jenv, nullptr);
    });

    //RELEASE_JOBJECT(help_cls);

}

JniHelper::~JniHelper()
{
}

void JniHelper::destroy(JNIEnv* env)
{
    msg_loop().post_task([this]() {
        jvm->DetachCurrentThread();
    });

    RELEASE_JOBJECT(help_cls);

    RELEASE_JOBJECT(help_cls);
}
