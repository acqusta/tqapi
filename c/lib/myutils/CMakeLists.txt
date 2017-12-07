project(myutils)

set(SRC 
  mprpc.cpp
  mprpc.h
  mprpc_socket.cpp
  mprpc_socket.cpp~
  mprpc_socket.h
  socket_connection.cpp
  socket_connection.h
  socketutils.h
  stringutils.h
)

include_directories( . )

add_library(myutils STATIC ${SRC})

target_link_libraries(myutils msgpack glog-static)
