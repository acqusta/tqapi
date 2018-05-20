project(tqslib)

set(SRC
  bt/backtest.cpp
  bt/backtest.h
  rt/realtime.cpp
  rt/realtime.h
  bt/sim_context.cpp
  bt/sim_context.h
  bt/sim_data.cpp
  bt/sim_data.h
  bt/sim_trade.cpp
  bt/sim_trade.h
  algo/algo_pm.cpp
  algo/algo_pm.h
  algo/algo_twap.cpp
  algo/algo_vwap.cpp
  stralet.cpp
  stralet.h
  tqs_cs.cpp
)

include_directories(.)
include_directories(../api)

add_library(tqs-static STATIC ${SRC})
target_link_libraries(tqs-static jsoncpp)

add_library(tqs SHARED ${SRC})

target_link_libraries(tqs jsoncpp myutils tqapi)

add_executable(tqs_demo 
  bt/demo.cpp
  bt/demo_rbreaker.cpp
  bt/demo_doublema.cpp
  bt/demo_ifhft.cpp
)

target_link_libraries(tqs_demo
  tqapi-static
  tqs-static
)

install(TARGETS tqs_demo RUNTIME DESTINATION bin)
install(TARGETS tqs DESTINATION bin)