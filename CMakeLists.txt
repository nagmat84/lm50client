project(lm50client)

cmake_minimum_required( VERSION 2.6 )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY} )

find_package( Boost COMPONENTS system program_options )

include_directories( BEFORE . )

include_directories( ${Boost_INCLUDE_DIRS} )

message( "Boost include dir is ${Boost_INCLUDE_DIRS}" )
message( "Boost library dir is ${Boost_LIBRARIES}" )

IF( NOT CMAKE_BUILD_TYPE )
  SET( CMAKE_BUILD_TYPE "Debug" )
ENDIF()

SET( CMAKE_CXX_FLAGS "-pedantic -Wextra -Wall -Wno-c++0x-compat" )
SET( CMAKE_CXX_FLAGS_DEBUG "-pedantic -Wextra -Wall -Wno-c++0x-compat -DDEBUG -g -ggdb" )
SET( CMAKE_CXX_FLAGS_RELEASE "-pedantic -Wextra -Wall -Wno-c++0x-compat -DNDEBUG -O2" )

add_subdirectory( apps )

add_subdirectory( tests )

add_subdirectory( core EXCLUDE_FROM_ALL )

add_subdirectory( lib EXCLUDE_FROM_ALL )
