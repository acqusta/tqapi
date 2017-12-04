cmake_minimum_required(VERSION 2.8)
 
project(tqapi_pyext)

set(SRC
    tqapi_dapi.cpp
    tqapi_tapi.cpp
    tqapi_py.cpp
    tqapi_py.h
    tqapi.py
    test.py
)

include_directories( ../c ../lib )

if (WIN32)
  # add_definitions(-DPy_NO_ENABLE_SHARED)
  if (BUILD_WIN64)
    include_directories(
      D:/WinPython-64bit-2.7.10.3/python-2.7.10.amd64/include
      D:/WinPython-64bit-2.7.10.3/python-2.7.10.amd64/Lib/site-packages/numpy/core/include
      )
    link_directories(
      D:/WinPython-64bit-2.7.10.3/python-2.7.10.amd64/libs
      )
  else()
    include_directories(
      D:/WinPython-32bit-2.7.10.3/python-2.7.10/include
      D:/WinPython-32bit-2.7.10.3/python-2.7.10/Lib/site-packages/numpy/core/include
      )
    link_directories(
      D:/python2.7-win32/libs
      )
  endif()
  set(EXTRA_LIBS python27)

else()

  add_definitions(-fvisibility=hidden )

  include_directories(
    /usr/include/python2.7
    )
  set(EXTRA_LIBS python2.7 dl)
endif()

include_directories(
    .
    ../
    ${DEPS_DIR}
    ${DEPS_DIR}/zmq
    )

link_directories(
  ${DEPS_DIR}/x1/linux64
)


add_library(tqapi_py SHARED ${SRC})

target_link_libraries(tqapi_py
    tqapi
    loop
    ${COMMON_LIB}
    ${EXTRA_LIBS}
)

add_executable(tqapi_py_test ${SRC})

target_compile_definitions( tqapi_py_test PRIVATE BUILD_API_TEST=1)

target_link_libraries(tqapi_py_test
    tqapi
    loop
    ${COMMON_LIB}
    ${EXTRA_LIBS}
)

install(TARGETS tqapi_py DESTINATION ${LIB_PATH})

install(FILES tqapi.py DESTINATION ${LIB_PATH})

if (UNIX)
  install(FILES
    ${PROJECT_BINARY_DIR}/libtqapi_py.so
    DESTINATION ${LIB_PATH}
    RENAME _tqapi.so)
else()
  install(FILES
    ${PROJECT_BINARY_DIR}/../${CMAKE_INSTALL_PREFIX}/bin/tqapi_py.dll
    DESTINATION ${LIB_PATH}
    RENAME _tqapi.pyd)
endif()

