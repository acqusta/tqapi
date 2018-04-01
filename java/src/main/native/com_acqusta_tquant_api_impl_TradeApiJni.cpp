#include <stdarg.h>

#include "com_acqusta_tquant_api_impl_TradeApiJni.h"
#include "tquant_api.h"
#include "tqapi_jni.h"

using namespace std;
using namespace tquant::api;

jobject convert_order(JNIEnv* env, jclass help_cls, jmethodID createOrder, const tquant::api::Order* ord)
{
    return env->CallStaticObjectMethod(help_cls, createOrder,
        LocalRef(env, env->NewStringUTF(ord->account_id.c_str())    ).m_obj,       
        LocalRef(env, env->NewStringUTF(ord->code.c_str())          ).m_obj,
        LocalRef(env, env->NewStringUTF(ord->name.c_str())          ).m_obj,
        LocalRef(env, env->NewStringUTF(ord->entrust_no.c_str())    ).m_obj,
        LocalRef(env, env->NewStringUTF(ord->entrust_action.c_str())).m_obj,
        ord->entrust_date,
        ord->entrust_time,
        ord->entrust_price,
        ord->entrust_size,
        ord->fill_price,
        ord->fill_size,
        LocalRef(env, env->NewStringUTF(ord->status.c_str())      ).m_obj,
        LocalRef(env, env->NewStringUTF(ord->status_msg.c_str())  ).m_obj,
        ord->order_id);
}

jobject convert_trade(JNIEnv* env, jclass help_cls, jmethodID createTrade, const tquant::api::Trade* trd)
{
    return env->CallStaticObjectMethod(help_cls, createTrade,
        LocalRef(env, env->NewStringUTF(trd->account_id.c_str())         ).m_obj,
        LocalRef(env, env->NewStringUTF(trd->code.c_str())               ).m_obj,
        LocalRef(env, env->NewStringUTF(trd->name.c_str())               ).m_obj,
        LocalRef(env, env->NewStringUTF(trd->entrust_no.c_str())         ).m_obj,
        LocalRef(env, env->NewStringUTF(trd->entrust_action.c_str())     ).m_obj,
        LocalRef(env, env->NewStringUTF(trd->fill_no.c_str())            ).m_obj,
        trd->fill_price,
        trd->fill_size,
        trd->fill_date,
        trd->fill_time);
}

jobject convert_account_info(JNIEnv* env, jclass help_cls, jmethodID createAccountInfo, const tquant::api::AccountInfo* act)
{
    return env->CallStaticObjectMethod(help_cls, createAccountInfo,
            LocalRef (env, env->NewStringUTF(act->account_id.c_str())   ).m_obj,
            LocalRef (env, env->NewStringUTF(act->broker.c_str())       ).m_obj,
            LocalRef (env, env->NewStringUTF(act->account.c_str())      ).m_obj,
            LocalRef (env, env->NewStringUTF(act->status.c_str())       ).m_obj,
            LocalRef (env, env->NewStringUTF(act->msg.c_str())          ).m_obj,
            LocalRef (env, env->NewStringUTF(act->account_type.c_str()) ).m_obj);
}

/*
 * Class:     com_acqusta_tquant_api_impl_TradeApiJni
 * Method:    queryAccountStatus
 * Signature: (J)[Lcom/acqusta/tquant/api/TradeApi/AccountInfo;
 */
JNIEXPORT jobjectArray JNICALL Java_com_acqusta_tquant_api_impl_TradeApiJni_queryAccountStatus
(JNIEnv * env, jclass, jlong h)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    auto r = wrap->api->trade_api()->query_account_status();
    if (!r.value) {
        throwJavaException(env, "%s", r.msg.c_str());
        return 0;
    }

    try {
        jobjectArray arr = env->NewObjectArray((jsize)r.value->size(), env->FindClass("Lcom/acqusta/tquant/api/TradeApi$AccountInfo;"), nullptr);
        for (size_t i = 0; i < r.value->size(); i++) {
            auto obj = convert_account_info(env, wrap->help_cls, wrap->createAccountInfo, &r.value->at(i));
            env->SetObjectArrayElement(arr, (jsize)i, obj);
            env->DeleteLocalRef(obj);
        }

        return arr;
    }
    catch (const exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return 0;
    }
}

/*
 * Class:     com_acqusta_tquant_api_impl_TradeApiJni
 * Method:    queryBalance
 * Signature: (JLjava/lang/String;)Lcom/acqusta/tquant/api/TradeApi/Balance;
 */
JNIEXPORT jobject JNICALL Java_com_acqusta_tquant_api_impl_TradeApiJni_queryBalance
  (JNIEnv *env, jclass, jlong h, jstring account_id)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    auto r = wrap->api->trade_api()->query_balance(get_string(env, account_id));
    if (!r.value) {
        throwJavaException(env, "%s", r.msg.c_str());
        return 0;
    }

    try {
        auto& bal = r.value;

        jobject obj = env->CallStaticObjectMethod(wrap->help_cls, wrap->createBalance,
            env->NewStringUTF(bal->account_id.c_str()),
            env->NewStringUTF(bal->fund_account.c_str()),
            bal->init_balance,
            bal->enable_balance,
            bal->margin,
            bal->float_pnl,
            bal->close_pnl);

        return obj;
    }
    catch (const exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}

/*
 * Class:     com_acqusta_tquant_api_impl_TradeApiJni
 * Method:    queryOrders
 * Signature: (JLjava/lang/String;)[Lcom/acqusta/tquant/api/TradeApi/Order;
 */
JNIEXPORT jobjectArray JNICALL Java_com_acqusta_tquant_api_impl_TradeApiJni_queryOrders
  (JNIEnv * env, jclass, jlong h, jstring account_id)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    auto r = wrap->api->trade_api()->query_orders(get_string(env, account_id));
    if (!r.value) {
        throwJavaException(env, "%s", r.msg.c_str());
        return 0;
    }

    try {

        jobjectArray arr = env->NewObjectArray((jsize)r.value->size(), env->FindClass("Lcom/acqusta/tquant/api/TradeApi$Order;"), nullptr);
        for (size_t i = 0; i < r.value->size(); i++) {
            auto ord = convert_order(env, wrap->help_cls, wrap->createOrder, &r.value->at(i));
            env->SetObjectArrayElement(arr, (jsize)i, ord);
            env->DeleteLocalRef(ord);
        }

        return arr;
    }
    catch (const exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_TradeApiJni
 * Method:    queryTrades
 * Signature: (JLjava/lang/String;)[Lcom/acqusta/tquant/api/TradeApi/Trade;
 */
JNIEXPORT jobjectArray JNICALL Java_com_acqusta_tquant_api_impl_TradeApiJni_queryTrades
    (JNIEnv * env, jclass, jlong h, jstring account_id)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    auto r = wrap->api->trade_api()->query_trades(get_string(env, account_id));
    if (!r.value) {
        throwJavaException(env, "%s", r.msg.c_str());
        return 0;
    }

    try {

        jobjectArray arr = env->NewObjectArray((jsize)r.value->size(), env->FindClass("Lcom/acqusta/tquant/api/TradeApi$Trade;"), nullptr);
        for (size_t i = 0; i < r.value->size(); i++) {
            auto obj = convert_trade(env, wrap->help_cls, wrap->createTrade, &r.value->at(i));
            env->SetObjectArrayElement(arr, (jsize)i, obj);
            env->DeleteLocalRef(obj);
        }

        return arr;
    }
    catch (const exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_TradeApiJni
 * Method:    queryPositions
 * Signature: (JLjava/lang/String;)[Lcom/acqusta/tquant/api/TradeApi/Position;
 */
JNIEXPORT jobjectArray JNICALL Java_com_acqusta_tquant_api_impl_TradeApiJni_queryPositions
    (JNIEnv * env, jclass, jlong h, jstring account_id)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }

    auto r = wrap->api->trade_api()->query_positions(get_string(env, account_id));
    if (!r.value) {
        throwJavaException(env, "%s", r.msg.c_str());
        return 0;
    }

    try {
        jobjectArray arr = env->NewObjectArray((jsize)r.value->size(), env->FindClass("Lcom/acqusta/tquant/api/TradeApi$Position;"), nullptr);
        for (size_t i = 0; i < r.value->size(); i++) {
            auto ord = &r.value->at(i);
            jobject obj = env->CallStaticObjectMethod(wrap->help_cls, wrap->createPosition,
                env->NewStringUTF(ord->account_id.c_str()),
                env->NewStringUTF(ord->code.c_str()) ,
                env->NewStringUTF(ord->name.c_str()) ,
                ord->current_size                    ,
                ord->enable_size                     ,
                ord->init_size                       ,
                ord->today_size                      ,
                ord->frozen_size                     ,
                env->NewStringUTF(ord->side.c_str()) ,
                ord->cost                            ,
                ord->cost_price                      ,
                ord->last_price                      ,
                ord->float_pnl                       ,
                ord->close_pnl                       ,
                ord->margin                          ,
                ord->commission                      );

            env->SetObjectArrayElement(arr, (jsize)i, obj);
        }

        env->DeleteLocalRef(wrap->help_cls);
        return arr;
    }
    catch (const exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_TradeApiJni
 * Method:    placeOrder
 * Signature: (JLjava/lang/String;Ljava/lang/String;DJLjava/lang/String;I)Lcom/acqusta/tquant/api/TradeApi/OrderID;
 */
JNIEXPORT jobject JNICALL Java_com_acqusta_tquant_api_impl_TradeApiJni_placeOrder
  (JNIEnv *env, jclass cls, jlong h, jstring account_id, jstring code, jdouble price, jlong size, jstring action, jint order_id)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }
    try {
        string s_account_id = get_string(env, account_id);
        string s_code       = get_string(env, code);
        string s_action     = get_string(env, action);

        auto r = wrap->api->trade_api()->place_order(s_account_id, s_code, price, size, s_action, order_id);
        if (!r.value) {
            throwJavaException(env, "%s", r.msg.c_str());
            return 0;
        }

        return env->CallStaticObjectMethod(wrap->help_cls, wrap->createOrderID, r.value->entrust_no.c_str(), r.value->order_id);
    }
    catch (const exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_TradeApiJni
 * Method:    cancelOrder
 * Signature: (JLjava/lang/String;Ljava/lang/String;I)Ljava/lang/Boolean;
 */
JNIEXPORT jboolean JNICALL Java_com_acqusta_tquant_api_impl_TradeApiJni_cancelOrder__JLjava_lang_String_2Ljava_lang_String_2I
  (JNIEnv *env, jclass cls, jlong h, jstring account_id, jstring code, jint order_id)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }
    try {
        string s_account_id = get_string(env, account_id);
        string s_code = get_string(env, code);

        auto r = wrap->api->trade_api()->cancel_order(s_account_id, s_code, order_id);
        if (!r.value) {
            throwJavaException(env, "%s", r.msg.c_str());
            return 0;
        }
        return *r.value ? JNI_TRUE : JNI_FALSE;
    }
    catch (const exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return JNI_FALSE;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_TradeApiJni
 * Method:    cancelOrder
 * Signature: (JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Boolean;
 */
JNIEXPORT jboolean JNICALL Java_com_acqusta_tquant_api_impl_TradeApiJni_cancelOrder__JLjava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jclass cls, jlong h, jstring account_id, jstring code, jstring entrust_no)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }
    try {
        string s_account_id = get_string(env, account_id);
        string s_code = get_string(env, code);
        string s_entrust_no = get_string(env, entrust_no);

        auto r = wrap->api->trade_api()->cancel_order(s_account_id, s_code, s_entrust_no);
        if (!r.value) {
            throwJavaException(env, "%s", r.msg.c_str());
            return 0;
        }
        return *r.value ? JNI_TRUE : JNI_FALSE;
    }
    catch (const exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return JNI_FALSE;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_TradeApiJni
 * Method:    query
 * Signature: (JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_acqusta_tquant_api_impl_TradeApiJni_query
  (JNIEnv * env, jclass cls, jlong h, jstring account_id, jstring command, jstring params)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return 0;
    }
    try {
        string s_account_id = get_string(env, account_id);
        string s_command = get_string(env, command);
        string s_params = get_string(env, params);

        auto r = wrap->api->trade_api()->query(s_account_id, s_command, s_params);
        if (!r.value) {
            throwJavaException(env, "%s", r.msg.c_str());
            return 0;
        }
        return env->NewStringUTF(r.value->c_str());
    }
    catch (const exception& e) {
        throwJavaException(env, "exception: %s", e.what());
        return nullptr;
    }
}


/*
 * Class:     com_acqusta_tquant_api_impl_TradeApiJni
 * Method:    setCallback
 * Signature: (JLcom/acqusta/tquant/api/TradeApi/Callback;)V
 */
JNIEXPORT void JNICALL Java_com_acqusta_tquant_api_impl_TradeApiJni_setCallback
    (JNIEnv * env, jclass cls, jlong h, jobject callback)
{
    auto wrap = reinterpret_cast<TQuantApiWrap*>(h);
    if (!wrap) {
        throwJavaException(env, "null handle");
        return;
    }

    if (callback) {
        jclass new_callback_class = env->GetObjectClass(callback);
        auto onOrderStatus = env->GetMethodID(new_callback_class, "onOrderStatus",
            "(Lcom/acqusta/tquant/api/TradeApi$Order;)V");

        if (!onOrderStatus) {
            throwJavaException(env, "No onOrderStatus in Callback");
            return;
        }

        auto onOrderTrade = env->GetMethodID(new_callback_class, "onOrderTrade",
            "(Lcom/acqusta/tquant/api/TradeApi$Trade;)V");

        if (!onOrderTrade) {
            throwJavaException(env, "No onOrderTrade in TataApi$Callback");
            return;
        }

        auto onAccountStatus = env->GetMethodID(new_callback_class, "onAccountStatus",
            "(Lcom/acqusta/tquant/api/TradeApi$AccountInfo;)V");

        if (!onAccountStatus) {
            throwJavaException(env, "No onAccountStatus in TataApi$Callback");
            return;
        }

        callback = env->NewGlobalRef(callback);
        wrap->m_tapi_loop.msg_loop().PostTask([wrap, callback, onOrderStatus, onOrderTrade, onAccountStatus]() {
            if (wrap->tapi_callback)
                wrap->tapi_jenv->DeleteGlobalRef(wrap->tapi_callback);
            wrap->tapi_callback      = callback;
            wrap->tapi_onOrderStatus = onOrderStatus;
            wrap->tapi_onOrderTrade  = onOrderTrade;
            wrap->tapi_onAccountStatus = onAccountStatus;
        });
    }
    else {
        wrap->m_tapi_loop.msg_loop().PostTask([wrap]() {
            if (wrap->tapi_callback) {
                wrap->tapi_jenv->DeleteGlobalRef(wrap->tapi_callback);
                wrap->tapi_callback = nullptr;
                wrap->tapi_onOrderStatus = nullptr;
                wrap->tapi_onOrderTrade  = nullptr;
                wrap->tapi_onAccountStatus = nullptr;
            }
        });
    }

}


void TQuantApiWrap::on_order_status(shared_ptr<Order> order)
{
    if (!tapi_callback) return;

    m_tapi_loop.msg_loop().PostTask([this, order]() {
        try {
            auto ord = convert_order(tapi_jenv, this->help_cls, createBar, order.get());
            this->tapi_jenv->CallVoidMethod(tapi_callback, this->tapi_onOrderStatus, ord);
            tapi_jenv->DeleteLocalRef(ord);
        }
        catch (const exception& e) {
            throwJavaException(this->tapi_jenv, "exception: ", e.what());
        }
    });
}

void TQuantApiWrap::on_order_trade(shared_ptr<Trade> trade)
{
    if (!tapi_callback) return;

    m_tapi_loop.msg_loop().PostTask([this, trade]() {
        try {
            auto trd = convert_trade(tapi_jenv, this->help_cls, createTrade, trade.get());
            this->tapi_jenv->CallVoidMethod(tapi_callback, this->tapi_onOrderTrade, trd);
            tapi_jenv->DeleteLocalRef(trd);
        }
        catch (const exception& e) {
            throwJavaException(this->tapi_jenv, "exception: ", e.what());
        }
    });
}

void TQuantApiWrap::on_account_status(shared_ptr<AccountInfo> account)
{
    if (!tapi_callback) return;

    m_tapi_loop.msg_loop().PostTask([this, account]() {
        try {
            auto ord = convert_account_info(tapi_jenv, this->help_cls, createAccountInfo, account.get());
            this->tapi_jenv->CallVoidMethod(tapi_callback, this->tapi_onAccountStatus, ord);
            tapi_jenv->DeleteLocalRef(ord);
        }
        catch (const exception& e) {
            throwJavaException(this->tapi_jenv, "exception: ", e.what());
        }
    });
}
