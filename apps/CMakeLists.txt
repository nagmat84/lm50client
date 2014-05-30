add_executable( lm50client lm50client.cpp program_options.cpp mode_human.cpp mode_cacti.cpp mode_daemon.cpp daemon_worker.cpp worker_rrd.cpp )
target_link_libraries( lm50client lm50core modbus ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
