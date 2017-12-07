project(api_c)

set(SRC
    tquant_api.h
    impl_tquant_api.h
    impl_tquant_api.cpp
    impl_data_api.h
    impl_trade_api.h
)

add_library(tqapi STATIC ${SRC})

target_link_libraries(tqapi
  myutils
  msgpack
  snappy
  glog-static
  ${COMMON_LIB}
)

set(SRC
    tquant_api_test.cpp
)
add_executable(tqapi_test ${SRC})

target_link_libraries(tqapi_test tqapi)

install(TARGETS tqapi_test DESTINATION bin)
