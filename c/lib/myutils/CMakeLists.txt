project(myutils)

set(SRC 
    zmq_connection.cpp
    zmq_connection.h
    zmq.hpp
    jsonrpc.cpp
    jsonrpc.h
)

include_directories( . )

add_library(myutils STATIC ${SRC})
