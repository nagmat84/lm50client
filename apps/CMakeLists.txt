add_executable( lm50client lm50client.cpp program_options.cpp mode_human.cpp mode_cacti.cpp )
#set_property( TARGET lm50tcp APPEND PROPERTY COMPILE_FLAGS "-pedantic -Wextra -Wall" )
target_link_libraries( lm50client lm50core modbus ${Boost_LIBRARIES} )