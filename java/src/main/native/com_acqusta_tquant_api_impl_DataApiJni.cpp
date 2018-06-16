#include "com_acqusta_tquant_api_impl_DataApiJni.h"
#include "tquant_api.h"
#include "tqapi_jni.h"

using namespace std;
using namespace tquant::api;

jobject convert_quote(JNIEnv* env, jclass help_cls, jmethodID createMarketQuote, const tquant::api::RawMarketQuote* q)
{    
    return env->CallStaticObjectMethod(help_cls, createMarketQuote,
        LocalRef(env, env->NewStringUTF(q->code)).m_obj,
        q->date,
        q->time,
        q->trading_day,
        q->open,
        q->high,
        q->low,
        q->close,
        q->last,
        q->high_limit,
        q->low_limit,
        q->pre_close,
        q->volume,
        q->turnover,
        q->ask1,
        q->ask2,
        q->ask3,
        q->ask4,
        q->ask5,
        0.0, //tick->ask6         ,
        0.0, //tick->ask7         ,
        0.0, //tick->ask8         ,
        0.0, //tick->ask9         ,
        0.0, //tick->ask10        ,
        q->bid1,
        q->bid2,
        q->bid3,
        q->bid4,
        q->bid5,
        0.0, //tick->bid6         ,
        0.0, //tick->bid7         ,
        0.0, //tick->bid8         ,
        0.0, //tick->bid9         ,
        0.0, //tick->bid10        ,
        q->ask_vol1,
        q->ask_vol2,
        q->ask_vol3,
        q->ask_vol4,
        q->ask_vol5,
        0, // tick->ask_vol6     ,
        0, // tick->ask_vol7     ,
        0, // tick->ask_vol8     ,
        0, // tick->ask_vol9     ,
        0, // tick->ask_vol10    ,
        q->bid_vol1,
        q->bid_vol2,
        q->bid_vol3,
        q->bid_vol4,
        q->bid_vol5,
        0, // tick->bid_vol6     ,
        0, // tick->bid_vol7     ,
        0, // tick->bid_vol8     ,
        0, // tick->bid_vol9     ,
        0, // tick->bid_vol10    ,
        q->settle,
        q->pre_settle,
        q->oi,
        q->pre_oi);

}


jobject convert_bar(JNIEnv* env, jclass help_cls, jmethodID createBar, const tquant::api::RawBar* bar)
{
    return env->CallStaticObjectMethod(help_cls, createBar,
                    LocalRef(env, env->NewStringUTF(bar->code)).m_obj,
                    bar->date        ,
                    bar->time        ,
                    bar->trading_day ,
                    bar->open        ,
                    bar->high        ,
                    bar->low         ,
                    bar->close       ,
                    bar->volume      ,
                    bar->turnover    ,
                    bar->oi          );
}

JNIEXPORT jlong JNICALL Java_com_acqusta_tquant_api_impl_DataApiJni_create
    (JNIEnv *env, jclass cls, jstring addr)
{
    try {
        const char* cs = env->GetStringUTFChars(addr, 0);
        if (!cs)
            return 0;
        string s(cs);
        env->ReleaseStringUTFChars(addr, cs);
        auto api = create_data_api(s);
        if (api) {
            auto wrap = new DataApiWrap(api, env);
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
* Class:     com_acqusta_tquant_api_impl_DataApiJni
* Method:    destroy
* Signature: (J)V
*/
JNIEXPORT void JNICALL Java_com_acqusta_tquant_api_impl_DataApiJni_destroy
    (JNIEnv * env, jclass cls, jlong h)
{
    if (h) {
        auto wrap = reinterpret_cast<DataApiWrap*>(h);
        if (!wrap) {
            throwJavaException(env, "null handle");
            return;
        }
        wrap->destroy(env);
        delete wrap;
    }
}

/*
 * Class:     com_acqusta_tquant_api_impl_DataApiJni
 * Method:    getTick
 * Signature: (JLjava/lang/String;I)[Lcom/acqusta/tquant/api/DataApi/MarketQuote;
 */
JNIEXPORT jobjectArray JNICALL Java_com_acqusta_tquant_api_impl_DataApiJni_getTick
    (JNIEnv * env, jclass cls, jlong h, jstring code, jint trading_day)
{
    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    try {
        string s_code = get_string(env, code);
        auto r = wrap->m_dapi->tick(s_code, trading_day);
        if (!r.value) {
            throwJavaException(env, "%s", r.msg.c_str());
            return 0;
        }

        jobjectArray arr = env->NewObjectArray((jsize)r.value->size(), env->FindClass("Lcom/acqusta/tquant/api/DataApi$MarketQuote;"), nullptr);
        for (size_t i = 0; i < r.value->size(); i++) {
            auto obj = convert_quote(env, wrap->help_cls, wrap->createMarketQuote, &r.value->at(i));
            env->SetObjectArrayElement(arr, (jsize)i, obj);
            env->DeleteLocalRef(obj);
        }

        return arr;
    }
    catch (const std::exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}

/*
 * Class:     com_acqusta_tquant_api_impl_DataApiJni
 * Method:    getBar
 * Signature: (JLjava/lang/String;Ljava/lang/String;IZ)[Lcom/acqusta/tquant/api/DataApi/Bar;
 */
JNIEXPORT jobjectArray JNICALL Java_com_acqusta_tquant_api_impl_DataApiJni_getBar
  (JNIEnv *env, jclass cls, jlong h, jstring code, jstring cycle, jint trading_day, jboolean align)
{
    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    try {
        std::string s_code = get_string(env, code);
        std::string s_cycle = get_string(env, cycle);

        auto r = wrap->m_dapi->bar(s_code, s_cycle.c_str(), trading_day, align!=0);
        if (!r.value) {
            throwJavaException(env, "%s", r.msg.c_str());
            return 0;
        }

        jobjectArray arr = env->NewObjectArray((jsize)r.value->size(), env->FindClass("Lcom/acqusta/tquant/api/DataApi$Bar;"), nullptr);
        for (size_t i = 0; i < r.value->size(); i++) {
            auto obj = convert_bar(env, wrap->help_cls, wrap->createBar, &r.value->at(i));
            env->SetObjectArrayElement(arr, (jsize)i, obj);
            env->DeleteLocalRef(obj);
        }

        return arr;
    }
    catch (const std::exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_DataApiJni
 * Method:    getDailyBar
 * Signature: (JLjava/lang/String;Ljava/lang/String;Ljava/lang/Boolean;)[Lcom/acqusta/tquant/api/DataApi/DailyBar;
 */
JNIEXPORT jobjectArray JNICALL Java_com_acqusta_tquant_api_impl_DataApiJni_getDailyBar
  (JNIEnv *env, jclass cls, jlong h, jstring code, jstring price_adj, jboolean align)
{
    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    try {
        std::string s_code      = get_string(env, code);
        std::string s_price_adj = get_string(env, price_adj);

        auto r = wrap->m_dapi->daily_bar(s_code, s_price_adj.c_str(), align != 0);
        if (!r.value) {
            throwJavaException(env, "%s", r.msg.c_str());
            return 0;
        }

        jobjectArray arr = env->NewObjectArray((jsize)r.value->size(), env->FindClass("Lcom/acqusta/tquant/api/DataApi$DailyBar;"), nullptr);
        for (size_t i = 0; i < r.value->size(); i++) {
            auto bar = &r.value->at(i);
            jobject obj = env->CallStaticObjectMethod(wrap->help_cls, wrap->createDailyBar,
                                LocalRef(env, env->NewStringUTF(bar->code)).m_obj,
                                bar->date              ,
                                bar->open              ,
                                bar->high              ,
                                bar->low               ,
                                bar->close             ,
                                bar->volume            ,
                                bar->turnover          ,
                                bar->oi                ,
                                bar->settle            ,
                                bar->pre_close         ,
                                bar->pre_settle        );

            env->SetObjectArrayElement(arr, (jsize)i, obj);
            env->DeleteLocalRef(obj);
        }

        return arr;
    }
    catch (const std::exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_DataApiJni
 * Method:    getQuote
 * Signature: (JLjava/lang/String;)[Lcom/acqusta/tquant/api/DataApi/MarketQuote;
 */
JNIEXPORT jobject JNICALL Java_com_acqusta_tquant_api_impl_DataApiJni_getQuote
  (JNIEnv * env, jclass cls, jlong h, jstring code)
{
    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    try {
        std::string s_code = get_string(env, code);

        auto r = wrap->m_dapi->quote(s_code);
        if (!r.value) {
            throwJavaException(env, "%s", r.msg.c_str());
            return 0;
        }

        return convert_quote(env, wrap->help_cls, wrap->createMarketQuote, r.value.get());
    }
    catch (const std::exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_DataApiJni
 * Method:    subscribe
 * Signature: (J[Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_acqusta_tquant_api_impl_DataApiJni_subscribe
  (JNIEnv * env, jclass cls, jlong h, jobjectArray codes)
{
    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    try {
        vector<string> s_codes;
        if (codes != nullptr) {
            for (int i = 0; i < env->GetArrayLength(codes); i++) {
                auto tmp = env->GetObjectArrayElement(codes, i);
                s_codes.push_back(get_string(env, (jstring)tmp));
                env->DeleteLocalRef(tmp);
            }
        }

        auto r = wrap->m_dapi->subscribe(s_codes);
        if (!r.value) {
            throwJavaException(env, "%s", r.msg.c_str());
            return nullptr;
        }

        jclass string_cls = (jclass)env->FindClass("Ljava/lang/String;");
        if (!string_cls) {
            throwJavaException(env, "No class java/lang/String");
            return nullptr;
        }

        jobjectArray arr = env->NewObjectArray((jsize)r.value->size(), string_cls, nullptr);
        for (int i = 0; i < (int)r.value->size(); i++) {
            auto tmp = env->NewStringUTF(r.value->at(i).c_str());
            env->SetObjectArrayElement(arr, i, tmp);
            env->DeleteLocalRef(tmp);
        }
        env->DeleteLocalRef(string_cls);
        return arr;
    }
    catch (const std::exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_DataApiJni
 * Method:    unsubscribe
 * Signature: (J[Ljava/lang/String;)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_acqusta_tquant_api_impl_DataApiJni_unsubscribe
  (JNIEnv * env, jclass cls, jlong h, jobjectArray codes)
{
    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    try {
        vector<string> s_codes;
        if (codes != nullptr) {
            for (int i = 0; i < env->GetArrayLength(codes); i++) {
                auto tmp = env->GetObjectArrayElement(codes, i);
                s_codes.push_back(get_string(env, (jstring)tmp));
                env->DeleteLocalRef(tmp);
            }
        }

        auto r = wrap->m_dapi->unsubscribe(s_codes);
        if (!r.value) {
            throwJavaException(env, "%s", r.msg.c_str());
            return nullptr;
        }

        jclass string_cls = (jclass)env->FindClass("Ljava/lang/String;");
        if (!string_cls) {
            throwJavaException(env, "No class java/lang/String");
            return nullptr;
        }

        jobjectArray arr = env->NewObjectArray((jsize)r.value->size(), string_cls, nullptr);
        for (int i = 0; i < (int)r.value->size(); i++) {
            auto tmp = env->NewStringUTF(r.value->at(i).c_str());
            env->SetObjectArrayElement(arr, i, tmp);
            env->DeleteLocalRef(tmp);
        }
        return arr;
    }
    catch (const std::exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_DataApiJni
 * Method:    setCallback
 * Signature: (JLcom/acqusta/tquant/api/DataApi/Callback;)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_api_impl_DataApiJni_setCallback
  (JNIEnv * env, jclass cls, jlong h, jobject callback)
{
    auto wrap = reinterpret_cast<DataApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return;
    }

    if (callback) {
        jclass new_callback_class = env->GetObjectClass(callback);
        jmethodID onMarketQuote = env->GetMethodID(new_callback_class, "onMarketQuote",
            "(Lcom/acqusta/tquant/api/DataApi$MarketQuote;)V");

        if (!onMarketQuote) {
            throwJavaException(env, "No onMarketQuote in Callback");
            return;
        }
        jmethodID onBar = env->GetMethodID(new_callback_class, "onBar",
            "(Ljava/lang/String;Lcom/acqusta/tquant/api/DataApi$Bar;)V");

        if (!onBar) {
            throwJavaException(env, "No onBar in Callback");
            return;
        }

        callback = env->NewGlobalRef(callback);
        wrap->msg_loop().PostTask([wrap, callback, onMarketQuote, onBar]() {
            if (wrap->m_dapi_callback)
                wrap->jenv->DeleteGlobalRef(wrap->m_dapi_callback);

            wrap->m_dapi_callback = callback;
            wrap->dapi_onMarketQuote = onMarketQuote;
            wrap->dapi_onBar = onBar;
        });
    }
    else {
        wrap->msg_loop().PostTask([wrap]() {
            if (wrap->m_dapi_callback) {
                wrap->jenv->DeleteGlobalRef(wrap->m_dapi_callback);
                wrap->m_dapi_callback = nullptr;
                wrap->dapi_onMarketQuote = nullptr;
                wrap->dapi_onBar = nullptr;
            }
        });
    }
}

void DataApiWrap::on_market_quote(shared_ptr<const MarketQuote> quote)
{
    if (!m_dapi_callback) return;

    msg_loop().PostTask([this, quote]() {
        try {
            if (jenv) {
                auto q = convert_quote(jenv, help_cls, createMarketQuote, quote.get());
                jenv->CallVoidMethod(m_dapi_callback, dapi_onMarketQuote, q);
                jenv->DeleteLocalRef(q);
            }
        }
        catch (const exception& e) {
            throwJavaException(jenv, "exception: ", e.what());
        }
    });
}
void DataApiWrap::on_bar(const string& cycle, shared_ptr<const Bar> bar)
{
    if (!m_dapi_callback) return;

    msg_loop().PostTask([this, cycle, bar]() {
        try {
            if (jenv) {
                auto py_cycle = jenv->NewStringUTF(cycle.c_str());
                auto py_bar = convert_bar(jenv, help_cls, createBar, bar.get());
                jenv->CallVoidMethod(m_dapi_callback, dapi_onBar, cycle, py_bar);
                jenv->DeleteLocalRef(py_bar);
            }
        }
        catch (const exception& e) {
            throwJavaException(jenv, "exception: ", e.what());
        }
    });
}
