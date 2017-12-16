project(myutils)

set(SRC 
  ipc_connection.cpp
  ipc_connection.h
  mprpc.cpp
  mprpc.h
  shm_queue.cpp
  shm_queue.h
  socket_connection.cpp
  socket_connection.h
  socketutils.cpp
  socketutils.h
  stringutils.h
  filemapping.cpp
  filemapping.h
  misc.cpp
  misc.h
)

include_directories( . )

add_library(myutils STATIC ${SRC})

target_link_libraries(myutils msgpack loop)
