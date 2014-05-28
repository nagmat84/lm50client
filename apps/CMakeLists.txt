add_executable( lm50client lm50client.cpp program_options.cpp mode_human.cpp mode_cacti.cpp mode_daemon.cpp )
target_link_libraries( lm50client lm50core modbus ${Boost_LIBRARIES} )
