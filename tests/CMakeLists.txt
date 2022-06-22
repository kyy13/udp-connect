# udp-connect
# Kyle J Burgess

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_subdirectory(test_udp_send_recv_ipv4)
add_subdirectory(test_udp_send_recv_ipv6)
add_subdirectory(test_connect_ipv4)
add_subdirectory(test_connect_ipv6)
