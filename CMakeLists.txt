cmake_minimum_required(VERSION 2.8)

project(tquant_api)

option(BUILD_WIN64  "Build Win64"     ON)
option(BUILD_JAVA   "Build Java"      OFF)
option(BUILD_PYTHON "Build Python"    ON)
option(BUILD_DOTNET "Build .Net Core" OFF)
option(BUILD_WIN_ON_LINUX "Build Win64 on Linux" OFF)



set(CMAKE_INSTALL_PREFIX dist)

include_directories(
  lib/
  lib/msgpack/include
  lib/jsoncpp/inc
)

add_definitions(
  -DGOOGLE_GLOG_DLL_DECL=
  -DGLOG_NO_ABBREVIATED_SEVERITIES
  -DZMQ_STATIC
  )

if (MSVC)
  add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
    -D_WINSOCK_DEPRECATED_NO_WARNINGS
#    -Dsnprintf=_snprintf
    -DNOMINMAX
    -DMSGPACK_DLLEXPORT=
#	/MT
  )

  set(BIN_PATH bin)
  set(LIB_PATH bin)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /source-charset:utf-8")
  
else()

  add_definitions(-fPIC )

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -std=c99")

  set(BIN_PATH bin)
  set(LIB_PATH lib)

endif()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -Wl,-Bstatic -lstdc++ -lwinpthread -Wl,-Bdynamic")

if (WIN32)
  set(COMMON_LIB ws2_32 iphlpapi)
elseif (APPLE)
  set(COMMON_LIB pthread )
else()
  set(COMMON_LIB pthread rt)
endif()


add_subdirectory(lib/msgpack)
add_subdirectory(lib/myutils)
add_subdirectory(lib/snappy)
add_subdirectory(lib/jsoncpp)

add_subdirectory(c)

if (BUILD_JAVA)
  add_subdirectory(java)
endif()

if (BUILD_PYTHON)
  add_subdirectory(pyext)
endif()

if (BUILD_DOTNET)
  add_subdirectory(cs)
endif()
