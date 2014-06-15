find_package( PkgConfig REQUIRED )
pkg_search_module( RRDTool librrd )
# Above line sets RRDTool_FOUND == 1 on success and
# RRDTool_LDFLAGS to correct linker argument

if( NOT RRDTool_FOUND )
	set( RRDTOOL_FOUND FALSE CACHE INTERNAL "" FORCE )
	return()
else()
	set( RRDTOOL_FOUND TRUE CACHE INTERNAL "" FORCE )
endif()

find_path( RRDTool_INCLUDE_DIR rrd.h /usr/include /usr/local/include )
if( RRDTool_INCLUDE_DIR )
	set( RRDTOOL_FOUND TRUE CACHE INTERNAL "" FORCE )
else()
	set( RRDTOOL_FOUND FALSE CACHE INTERNAL "" FORCE )
endif()
