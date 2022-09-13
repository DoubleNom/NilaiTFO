cmake_minimum_required(VERSION 3.14)
project(NilaiTFO_Test)

project(${projectName} C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

get_filename_component(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)

option(ENABLE_TEST "Enable testing" OFF)
if(ENABLE_TEST)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_compile_definitions(GTEST)

include(GoogleTest)
enable_testing()

add_compile_definitions(NILAI_TEST)
add_compile_definitions(STM32L452xx)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Mocks/STM32L452)

## Defines
option(TEST_DEFINES "Enable testing for defines" OFF)
if(TEST_DEFINES)
add_subdirectory(defines)
endif()

## Drivers
option(TEST_DRIVERS "Enable testing for drivers" OFF)
if(TEST_DRIVERS)
add_subdirectory(drivers)
option(TEST_DRIVERS_UART "Enable testing for Uart" OFF)
endif()

endif()