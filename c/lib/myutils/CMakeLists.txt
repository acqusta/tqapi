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
  ipc_connection.cpp
  ipc_connection.h
  )

include_directories( . )

add_library(myutils STATIC ${SRC})

if (UNIX)
  if (NOT APPLE) 
    target_link_libraries(myutils rt)
  endif()
endif()
