# udp-connect
# Kyle J Burgess

cmake_minimum_required(VERSION 3.7)
project(udpconnect)

option(BUILD_TESTS "build tests?" ON)

# library
add_library(
    ${PROJECT_NAME}
    SHARED
    src/UdcCommon.cpp
    src/UdcSocketSender.cpp
)

# includes
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    include
    ext/k13/include
)

IF (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
        -Wall
        -g
    )
ELSE()
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
        -O3
    )
ENDIF()

# c++ version
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)

# link libraries
target_link_libraries(
    ${PROJECT_NAME}
    Ws2_32
    -static-libgcc
    -static-libstdc++
    -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
)

IF(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
ENDIF()
