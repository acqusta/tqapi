Project(tqapi_jni)

add_subdirectory(src/main/native)


if (WIN32)
  add_custom_command(
    OUTPUT target/tquant-api-0.0.1-SNAPSHOT.jar
    COMMAND .\\build.bat
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
else()
  add_custom_command(
    OUTPUT target/tquant-api-0.0.1-SNAPSHOT.jar
    COMMAND sh ./build.sh
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

add_custom_target(build_java ALL DEPENDS
  target/tquant-api-0.0.1-SNAPSHOT.jar)

install(FILES
  target/tquant-api-0.0.1-SNAPSHOT-sources.jar
  target/tquant-api-0.0.1-SNAPSHOT.jar
  DESTINATION java)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/target/tquant-api-0.0.1-SNAPSHOT-sources.jar)

