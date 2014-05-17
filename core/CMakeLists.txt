add_library( lm50core STATIC lm50device.cpp program_options.cpp )
#set_property( TARGET modbus APPEND PROPERTY COMPILE_FLAGS "-pedantic -Wextra -Wall" )
