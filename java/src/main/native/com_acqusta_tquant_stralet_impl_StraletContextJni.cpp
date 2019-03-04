#include "com_acqusta_tquant_stralet_impl_StraletContextJni.h"

#include "tquant_api.h"
#include "stralet.h"
#include "tqapi_jni.h"

using namespace tquant::stralet;

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    getTradingDay
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_getTradingDay
  (JNIEnv *env, jclass cls, jlong h)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return 0;
    }

    return ctx->trading_day();
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    getCurTime
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_getCurTime
  (JNIEnv *env, jclass cls, jlong h)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return 0L;
    }

    auto dt = ctx->cur_time();
    return (int64_t)dt.date * 1000000000 + dt.time;
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    postEvent
 * Signature: (JLjava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_postEvent
  (JNIEnv *env, jclass cls, jlong h, jstring name, jlong data)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return;
    }

    string s_name = get_string(env, name);
    ctx->post_event(s_name.c_str(), (void*)data);
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    setTimer
 * Signature: (JJJJ)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_setTimer
  (JNIEnv *env, jclass cls, jlong h, jlong id, jlong delay, jlong data)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return;
    }

    ctx->set_timer(id, delay, (void*)data);
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    killTimer
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_killTimer
  (JNIEnv *env, jclass cls, jlong h, jlong id)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return;
    }

    ctx->kill_timer(id);
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    getDataApi
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_getDataApi
  (JNIEnv *env, jclass cls, jlong h)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return 0;
    }

    auto wrap = new DataApiWrap(ctx->data_api(), env, false);
    return reinterpret_cast<jlong>(wrap);
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    getTradeApi
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_getTradeApi
  (JNIEnv *env, jclass cls, jlong h)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return 0;
    }

    auto wrap = new TradeApiWrap(ctx->trade_api(), env, false);
    return reinterpret_cast<jlong>(wrap);
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    log
 * Signature: (JILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_log
  (JNIEnv *env, jclass cls, jlong h, jint severity, jstring msg)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return;
    }

    string s_msg = get_string(env, msg);
    ctx->logger((LogSeverity)severity) << s_msg;
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    getProperties
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_getProperties
  (JNIEnv *env, jclass cls, jlong h)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return nullptr;
    }

    string txt = ctx->get_properties();
    jstring str = env->NewStringUTF(txt.c_str());
    return str;
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    getMode
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_getMode
  (JNIEnv *env, jclass cls, jlong h)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return nullptr;
    }

    string txt = ctx->mode();
    jstring str = env->NewStringUTF(txt.c_str());
    return str;
}

/*
 * Class:     com_acqusta_tquant_stralet_impl_StraletContextJni
 * Method:    stop
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_stralet_impl_StraletContextJni_stop
  (JNIEnv *env, jclass cls, jlong h)
{
    auto ctx = reinterpret_cast<StraletContext*>(h);
    if (!ctx) {
        throwJavaException(env, "Wrong StraletContext handle");
        return;
    }
    ctx->stop();
}
