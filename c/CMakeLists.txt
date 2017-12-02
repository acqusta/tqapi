cmake_minimum_required(VERSION 2.8)

project(tquant_api)

set(CMAKE_INSTALL_PREFIX dist)


include_directories(
    lib/
    lib/msgpack/include
)

if (WIN32)
  add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
    -D_WINSOCK_DEPRECATED_NO_WARNINGS
    -Dsnprintf=_snprintf
    -DNOMINMAX
    -DGOOGLE_GLOG_DLL_DECL=
    -DGLOG_NO_ABBREVIATED_SEVERITIES
    -DZMQ_STATIC
  )

  include_directories(  
    D:/cpp/libs/zeromq-4.1.4/include
    D:/cpp/libs/glog-master/src/windows    
  )

  
  link_directories(
    D:/cpp/libs/glog-master/bin/vs2013/${CMAKE_VS_PLATFORM_NAME}/${CMAKE_INT_DIR}
    D:/cpp/libs/zeromq-4.1.4/bin/${CMAKE_VS_PLATFORM_NAME}/${CMAKE_INT_DIR}
  )

  set(COMMON_LIB
    libglog_static
    libzmq_static
#    libsodium_static
#    getopt
    ws2_32
  )

  set(LIB_PATH bin)

endif()

add_subdirectory(lib/loop)
add_subdirectory(lib/msgpack)
add_subdirectory(lib/myutils)
add_subdirectory(lib/snappy)

add_subdirectory(c)
#add_subdirectory(jni)
add_subdirectory(pyext)
