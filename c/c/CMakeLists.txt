project(api_c)

set(SRC
    tquant_api.h
    impl_tquant_api.h
    impl_tquant_api.cpp
    impl_data_api.h
    impl_trade_api.h
)

add_library(tquant_api STATIC ${SRC})

target_link_libraries(tquant_api
    msgpack
    myutils
    snappy
    ${COMMON_LIB}
)

set(SRC
    tquant_api_test.cpp
)
add_executable(tquant_api_test ${SRC})

target_link_libraries(tquant_api_test tquant_api)

install(TARGETS tquant_api_test DESTINATION bin)