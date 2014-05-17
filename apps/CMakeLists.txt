add_executable( lm50client lm50client.cpp )
#set_property( TARGET lm50tcp APPEND PROPERTY COMPILE_FLAGS "-pedantic -Wextra -Wall" )
target_link_libraries( lm50client lm50core modbus ${Boost_LIBRARIES} )