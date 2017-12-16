project(myutils)

set(SRC 
  mprpc.cpp
  mprpc.h
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

if (WIN32)
  list(APPEND SRC
    ipc_connection.cpp
    ipc_connection.h
    )
endif()

include_directories( . )

add_library(myutils STATIC ${SRC})

#target_link_libraries(myutils msgpack loop)
