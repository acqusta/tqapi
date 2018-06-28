project(api_c)

set(SRC
    tquant_api.h
    impl_tquant_api.h
    impl_tquant_api.cpp
    impl_data_api.h
    impl_trade_api.h
    tqapi_cs.cpp
    tqapi_cs.h
)

add_library(tqapi-static STATIC ${SRC})

if (UNIX)
  set(DL_LIB dl)
endif()

target_link_libraries(tqapi-static
  myutils
  msgpack
  snappy
  ${DL_LIB}
  ${COMMON_LIB}
)

if (NOT BUILD_TQS)
add_library(tqapi SHARED ${SRC})

target_link_libraries(tqapi
  myutils
  msgpack
  snappy
  ${DL_LIB}
  ${COMMON_LIB}
)

install(TARGETS tqapi DESTINATION bin)

install(TARGETS tqapi-static DESTINATION cpp)

target_compile_definitions(tqapi PRIVATE _TQAPI_DLL_IMPL=1)

set(SRC
  tquant_api_test.cpp
)

add_executable(tqapi_test ${SRC})

target_link_libraries(tqapi_test tqapi-static)

install(TARGETS tqapi_test DESTINATION bin)
endif()

install(FILES tquant_api.h DESTINATION cpp)
 
