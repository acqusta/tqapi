project(tqapi_jni)

include_directories(
  .
  ../../../../c/api
  ../../../../lib
  )

if (WIN32)
  include_directories(jvm_inc/win32)
else()
  include_directories(jvm_inc/unix)
endif()

set(SRC
  tqapi_jni.h
  tqapi_jni.cpp
  com_acqusta_tquant_api_impl_DataApiJni.cpp
  com_acqusta_tquant_api_impl_DataApiJni.h
  com_acqusta_tquant_api_impl_TradeApiJni.cpp
  com_acqusta_tquant_api_impl_TradeApiJni.h
  com_acqusta_tquant_api_impl_TQuantApiJni.cpp
  com_acqusta_tquant_api_impl_TQuantApiJni.h
  )

add_library(tqapi_jni SHARED ${SRC})

target_link_libraries(tqapi_jni tqapi-static)

install(TARGETS tqapi_jni 
    LIBRARY DESTINATION java
    RUNTIME DESTINATION java)    