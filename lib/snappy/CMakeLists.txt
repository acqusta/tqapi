project(snappy)


include_directories( . )

set(SRC 
	snappy.cc snappy-sinksource.cc snappy-stubs-internal.cc snappy-c.cc
	snappy.h snappy-sinksource.h snappy-stubs-public.h snappy-c.h
)

add_library(snappy STATIC ${SRC})
install(TARGETS snappy DESTINATION cpp)