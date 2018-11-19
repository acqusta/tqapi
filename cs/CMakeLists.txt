project(tqapi_cs)


#    COMMAND npm install
#    COMMAND bower install
#    COMMAND grunt --output_dir=${CMAKE_CURRENT_BINARY_DIR}/web2

if (WIN32)
  add_custom_command(
    OUTPUT TQuantApi.dll
    COMMAND .\\compile.bat ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
else()
  add_custom_command(
    OUTPUT TQuantApi.dll
    COMMAND ./compile.sh ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

add_custom_target(build_cs ALL DEPENDS TQuantApi.dll)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/TQuantApi.dll)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/TQuantApi.dll
  DESTINATION bin/ )

