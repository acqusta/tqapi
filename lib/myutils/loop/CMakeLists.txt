project(loop)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_INSTALL_PREFIX dist)

set(SRC
    AsyncCall.h
    MessageLoop.cpp
    MessageLoop.h
    RunLoop.cpp
    RunLoop.h
    MsgRunLoop.h
    MsgLoopFuture.h
)

include_directories(
    .
)

add_library(loop STATIC ${SRC})


