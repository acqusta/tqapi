project(tquant_api)

if (0)
  add_subdirectory(api)
  add_subdirectory(tqs)
endif()

include_directories(
  api
  tqs
  tqs/bt
  tqs/rt
)

set(SRC
  api/tquant_api.h
  api/impl_tquant_api.h
  api/impl_tquant_api.cpp
  api/impl_data_api.h
  api/impl_trade_api.h
  api/tqapi_cs.cpp
  api/tqapi_cs.h

  tqs/rt/realtime.cpp
  tqs/rt/realtime.h
  tqs/bt/backtest.cpp
  tqs/bt/backtest.h
  tqs/bt/sim_context.cpp
  tqs/bt/sim_context.h
  tqs/bt/sim_data.cpp
  tqs/bt/sim_data.h
  tqs/bt/sim_trade.cpp
  tqs/bt/sim_trade.h
  tqs/logger.h
  tqs/logger.cpp
  tqs/stralet.cpp
  tqs/stralet.h
  tqs/tqs_cs.cpp
)

if (UNIX)
  set (DL_LIB dl)
endif()

# Static library
add_library(tqapi-static STATIC ${SRC})
target_link_libraries(tqapi-static
  jsoncpp
  myutils
  msgpack
  snappy
  ${DL_LIB}
  ${COMMON_LIB}
)

# Shared Library
add_library(tqapi SHARED ${SRC})
target_link_libraries(tqapi
  jsoncpp
  myutils
  msgpack
  snappy
  ${DL_LIB}
  ${COMMON_LIB}
)
target_compile_definitions(tqapi PRIVATE _TQAPI_DLL_IMPL=1)

install(TARGETS tqapi DESTINATION bin)
install(TARGETS tqapi-static DESTINATION cpp)

install(FILES
  api/tquant_api.h
  DESTINATION cpp
)

install(FILES
  tqs/stralet.h
  tqs/logger.h
  DESTINATION cpp
)

install(FILES
  tqs/bt/backtest.h
  DESTINATION cpp/bt
)

install(FILES
  tqs/rt/realtime.h
  DESTINATION cpp/rt
)

# Test

add_executable(tqapi_test
  api/tquant_api_test.cpp
)

target_link_libraries(tqapi_test tqapi-static)

install(TARGETS tqapi_test DESTINATION bin)

# Demo

add_executable(tqs_demo 
  tqs/demo/demo.cpp
  tqs/demo/demo_rbreaker.cpp
  tqs/demo/demo_doublema.cpp
)

target_link_libraries(tqs_demo
  tqapi-static
)

install(TARGETS tqs_demo RUNTIME DESTINATION bin)
