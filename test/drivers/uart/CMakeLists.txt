include_directories(${ROOT_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/Mocks/STM32L452)
add_compile_options("SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/NilaiTFOConfig.h")
add_compile_options("SHELL:-include ${CMAKE_SOURCE_DIR}/Mocks/UART/uart.h")
add_compile_definitions(HAL_UART_MODULE_ENABLED)

add_executable(
        Uart_test
        test.cpp
        ${ROOT_DIR}/drivers/uart/module.cpp
        ${ROOT_DIR}/test/Mocks/STM32L452/stm32l4xx_hal.cpp
        ${ROOT_DIR}/test/Mocks/STM32L452/stm32l4xx_hal.cpp
        ${ROOT_DIR}/test/Mocks/UART/uart.cpp
        ${ROOT_DIR}/test/Mocks/DMA/dma.cpp
)

target_link_libraries(
        Uart_test
        gtest_main
)

gtest_discover_tests(Uart_test)