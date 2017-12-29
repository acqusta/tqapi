project(tqapi_jni)

include_directories(
  jvm_inc
  jvm_inc/win32
  .
  ../../../../c
  ../../../../lib
  )

set(SRC
  tqapi_jni.h
  com_acqusta_tquant_api_impl_DataApiJni.cpp
  com_acqusta_tquant_api_impl_DataApiJni.h
  com_acqusta_tquant_api_impl_TQuantApiJni.cpp
  com_acqusta_tquant_api_impl_TQuantApiJni.h
  com_acqusta_tquant_api_impl_TradeApiJni.cpp
  com_acqusta_tquant_api_impl_TradeApiJni.h
  )

add_library(tqapi_jni SHARED ${SRC})
target_link_libraries(tqapi_jni tqapi)