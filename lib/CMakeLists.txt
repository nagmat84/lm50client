add_library( modbus STATIC mb_ascii.cpp mb_base.cpp mb_constants.cpp mb_errorres.cpp mb_generic.cpp mb_rhregreq.cpp mb_rhregres.cpp mb_riregreq.cpp mb_riregres.cpp mb_tcpcomm.cpp mb_tcprar.cpp mb_typedefs.cpp mb_uint16.cpp mb_uint32.cpp )
#set_property( TARGET modbus APPEND PROPERTY COMPILE_FLAGS "-pedantic -Wextra -Wall" )
