# udp-connect
# Kyle J Burgess

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_subdirectory(test_connect_ipv4)
add_subdirectory(test_connect_ipv6)
add_subdirectory(test_timeout_ipv4)
add_subdirectory(test_timeout_ipv6)
add_subdirectory(test_reconnect_ipv4)
add_subdirectory(test_reconnect_ipv6)
add_subdirectory(test_unreliable_ipv4)
add_subdirectory(test_unreliable_ipv6)
add_subdirectory(test_reliable_ipv4)
add_subdirectory(test_reliable_ipv6)
