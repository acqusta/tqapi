#include "com_acqusta_tquant_api_impl_TQuantApiJni.h"
#include "tqapi_jni.h"
#include "tquant_api.h"

JNIEXPORT void JNICALL Java_com_acqusta_tquant_api_impl_TQuantApiJni_setParams
    (JNIEnv *env, jclass cls, jstring key, jstring value)
{
    string s_key = get_string(env, key);
    string s_value = get_string(env, value);

    tquant::api::set_params(s_key, s_value);
}
