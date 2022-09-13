option(TEST_DRIVERS_UART "Enable testing for UART" OFF)
if(TEST_DRIVERS_UART)
add_subdirectory(uart)
endif()