option(TEST_DEFINES_PINS "Enable testing for Pins" OFF)
if(TEST_DEFINES_PINS)
add_subdirectory(Pin)
endif()

option(TEST_DEFINES_BIT_MANIPULATION "Enable testing for Bit Manipulation" OFF)
if(TEST_DEFINES_BIT_MANIPULATION)
add_subdirectory(BitManipulations)
endif()

option(TEST_DEFINES_CIRCULAR_BUFFER "Enable testing for Circular Buffer" OFF)
if(TEST_DEFINES_CIRCULAR_BUFFER)
add_subdirectory(circular_buffer)
endif()