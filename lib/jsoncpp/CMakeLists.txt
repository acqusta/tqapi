project(lua-5.3)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_INSTALL_PREFIX dist)

set(SRC
    inc/json/allocator.h
    inc/json/assertions.h
    inc/json/autolink.h
    inc/json/config.h
    inc/json/features.h
    inc/json/forwards.h
    inc/json/json.h
    inc/json/reader.h
    inc/json/value.h
    inc/json/version.h
    inc/json/writer.h
    src/json_reader.cpp
    src/json_tool.h
    src/json_value.cpp
    src/json_writer.cpp
)

include_directories( inc src)

add_library(jsoncpp STATIC ${SRC})
install(TARGETS jsoncpp DESTINATION cpp)
