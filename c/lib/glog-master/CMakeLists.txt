cmake_minimum_required(VERSION 2.8)
    
project(glog)

set(CMAKE_INSTALL_PREFIX "dist")

set(SRC
  src/logging.cc
  src/raw_logging.cc
  src/utilities.cc
  src/vlog_is_on.cc
  src/symbolize.cc
  src/demangle.cc
  src/signalhandler.cc
  )

add_definitions(
  -DGOOGLE_GLOG_DLL_DECL=
  -DHAVE_CONFIG_H
  )

if (WIN32)
  add_definitions( -DHAVE_SNPRINTF=1 )

  list(APPEND SRC
    src/windows/port.cc
    )
  include_directories( src/windows )

else()
  if (APPLE)
    include_directories(
      tq
      tq/osx
      src
      )
  else()
    include_directories(
      tq
      tq/linux
      src
      )
  endif()
  add_definitions (
    -DHAVE_CONFIG_H
    -Wall -Wwrite-strings -Woverloaded-virtual -Wno-sign-compare
    -DNO_FRAME_POINTER
    -DNDEBUG
    #    -fvisibility=hidden
    )
endif()

add_library(glog-static STATIC ${SRC})

if (UNIX)
  target_link_libraries(glog-static pthread)
endif()

install(TARGETS glog-static DESTINATION lib)

if (UNIX)
  install(FILES
    src/glog/log_severity.h
    tq/glog/logging.h
    tq/glog/raw_logging.h
    tq/glog/stl_logging.h
    tq/glog/vlog_is_on.h
    DESTINATION include/glog)
else()
  install(FILES
    src/glog/log_severity.h
    src/windows/glog/logging.h
    src/windows/glog/raw_logging.h
    src/windows/glog/stl_logging.h
    src/windows/glog/vlog_is_on.h
    DESTINATION include/glog)
endif()
