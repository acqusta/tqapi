Project(tqapi_jni)

add_subdirectory(src/main/native)


if (WIN32)
  add_custom_command(
    OUTPUT target/tqapi.jar
    COMMAND .\\build.bat
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
else()
  add_custom_command(
    OUTPUT target/tqapi.jar
    COMMAND sh ./build.sh
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

add_custom_target(build_tqapi ALL DEPENDS target/tqapi.jar)

install(FILES target/tqapi.jar DESTINATION java)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/target/tqapi.jar)

