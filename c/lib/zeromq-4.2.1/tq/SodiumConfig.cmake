set(sodium_LIB_DIR         "${CMAKE_BINARY_DIR}/../../libsodium-1.0.12/build/dist/lib")

set(SODIUM_LIBRARIES sodium-static)

include_directories   (
  ${sodium_INCLUDE_DIR}
  ../libsodium-1.0.12/tq
  ../libsodium-1.0.12/src/libsodium/include
  ../libsodium-1.0.12/src/libsodium/include/sodium
  )
link_directories      (${sodium_LIB_DIR} )

add_definitions(
  -DZMQ_USE_LIBSODIUM=1
  -DSODIUM_STATIC
  -DSODIUM_EXPORT=
  )

if (UNIX)
  add_definitions(-fPIC)
endif()

set(SODIUM_FOUND True)
