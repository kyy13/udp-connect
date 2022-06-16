# udp-connect
# Kyle J Burgess

add_executable(
    test_udp_send_recv_ipv6
    src/main.cpp
)

target_include_directories(
    test_udp_send_recv_ipv6
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

IF (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(
        test_udp_send_recv_ipv6
        PRIVATE
        -Wall
        -g
    )
ELSE()
    target_compile_options(
        test_udp_send_recv_ipv6
        PRIVATE
        -O3
    )
ENDIF()

target_link_libraries(
    test_udp_send_recv_ipv6
    ${PROJECT_NAME}
    -Wl,-allow-multiple-definition
)

add_test(
    NAME
    test_udp_send_recv_ipv6
    COMMAND
    test_udp_send_recv_ipv6
)

set_target_properties(
    test_udp_send_recv_ipv6
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)
