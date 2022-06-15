# udp-connect
# Kyle J Burgess

add_executable(
    test_udp_send_recv
    src/main.cpp
)

target_include_directories(
    test_udp_send_recv
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

IF (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(
        test_udp_send_recv
        PRIVATE
        -Wall
        -g
    )
ELSE()
    target_compile_options(
        test_udp_send_recv
        PRIVATE
        -O3
    )
ENDIF()

target_link_libraries(
    test_udp_send_recv
    ${PROJECT_NAME}
    -Wl,-allow-multiple-definition
)

add_test(
    NAME
    test_udp_send_recv
    COMMAND
    test_udp_send_recv
)

set_target_properties(
    test_udp_send_recv
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)
