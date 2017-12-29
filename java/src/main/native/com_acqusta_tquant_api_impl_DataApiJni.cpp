#include "com_acqusta_tquant_api_impl_DataApiJni.h"
#include "tquant_api.h"
#include "tqapi_jni.h"

using namespace std;
using namespace tquant::api;

jobject convert_quote(JNIEnv* env, jclass help_cls, jmethodID createMarketQuote, tquant::api::MarketQuote* q)
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


jobject convert_bar(JNIEnv* env, jclass help_cls, jmethodID createBar, tquant::api::Bar* bar)
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

/*
 * Class:     com_acqusta_tquant_api_impl_DataApiJni
 * Method:    getTick
 * Signature: (JLjava/lang/String;I)[Lcom/acqusta/tquant/api/DataApi/MarketQuote;
 */
JNIEXPORT jobjectArray JNICALL Java_com_acqusta_tquant_api_impl_DataApiJni_getTick
(JNIEnv * env, jclass cls, jlong h, jstring code, jint trading_day)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    try {
        std::string s_code = get_string(env, code);
        auto r = wrap->api->data_api()->tick(s_code.c_str(), trading_day);
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
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    try {
        std::string s_code = get_string(env, code);
        std::string s_cycle = get_string(env, cycle);
        auto r = wrap->api->data_api()->bar(s_code.c_str(), s_cycle.c_str(), trading_day, align!=0);
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
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    try {
        std::string s_code      = get_string(env, code);
        std::string s_price_adj = get_string(env, price_adj);
        auto r = wrap->api->data_api()->daily_bar(s_code.c_str(), s_price_adj.c_str(), align != 0);
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
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    try {
        std::string s_code = get_string(env, code);
        auto r = wrap->api->data_api()->quote(s_code.c_str());
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
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
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
        auto r = wrap->api->data_api()->subscribe(s_codes);
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
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
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
        auto r = wrap->api->data_api()->unsubscribe(s_codes);
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
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
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
        auto r = wrap->api->data_api()->quote("000001.SH");
        if (r.value) {
            auto q = convert_quote(env, wrap->help_cls, wrap->createMarketQuote, r.value.get());
            env->CallVoidMethod(callback, onMarketQuote, q);
        }
        else {
            env->CallVoidMethod(new_callback_class, onMarketQuote, nullptr);
        }

        callback = env->NewGlobalRef(callback);
        wrap->m_dapi_loop.msg_loop().PostTask([wrap, callback, onMarketQuote, onBar]() {
            if (wrap->dapi_callback)
                wrap->dapi_jenv->DeleteGlobalRef(wrap->dapi_callback);

            wrap->dapi_callback = callback;
            wrap->dapi_onMarketQuote = onMarketQuote;
            wrap->dapi_onBar = onBar;
        });
    }
    else {
        wrap->m_dapi_loop.msg_loop().PostTask([wrap]() {
            if (wrap->dapi_callback) {
                wrap->dapi_jenv->DeleteGlobalRef(wrap->dapi_callback);
                wrap->dapi_callback = nullptr;
                wrap->dapi_onMarketQuote = nullptr;
                wrap->dapi_onBar = nullptr;
            }
        });
    }
}


void TQuantApiWrap::on_market_quote(shared_ptr<MarketQuote> quote)
{
    if (!dapi_callback) return;

    m_dapi_loop.msg_loop().PostTask([this, quote]() {
        try {
            if (dapi_jenv) {
                auto q = convert_quote(dapi_jenv, this->help_cls, createMarketQuote, quote.get());
                dapi_jenv->CallVoidMethod(this->dapi_callback, this->dapi_onMarketQuote, q);
                dapi_jenv->DeleteLocalRef(q);
            }
        }
        catch (const exception& e) {
            throwJavaException(this->dapi_jenv, "exception: ", e.what());
        }
    });
}
void TQuantApiWrap::on_bar(const char* cycle, shared_ptr<Bar> bar)
{
    if (!dapi_callback) return;

    string s_cycle(cycle);
    m_dapi_loop.msg_loop().PostTask([this, s_cycle, bar]() {
        try {
            if (dapi_jenv) {
                auto cycle = dapi_jenv->NewStringUTF(s_cycle.c_str());
                auto b = convert_bar(dapi_jenv, this->help_cls, createBar, bar.get());
                dapi_jenv->CallVoidMethod(this->dapi_callback, this->dapi_onBar, cycle, b);
                dapi_jenv->DeleteLocalRef(b);
            }
        }
        catch (const exception& e) {
            throwJavaException(this->dapi_jenv, "exception: ", e.what());
        }
    });
}
