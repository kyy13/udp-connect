# udp-connect
# Kyle J Burgess

add_executable(
    test_reliable_ipv4
    src/main.cpp
)

target_include_directories(
    test_reliable_ipv4
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

IF (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(
        test_reliable_ipv4
        PRIVATE
        -Wall
        -g
    )
ELSE()
    target_compile_options(
        test_reliable_ipv4
        PRIVATE
        -O3
    )
ENDIF()

target_link_libraries(
    test_reliable_ipv4
    ${PROJECT_NAME}
    -Wl,-allow-multiple-definition
)

add_test(
    NAME
    test_reliable_ipv4
    COMMAND
    test_reliable_ipv4
)

set_target_properties(
    test_reliable_ipv4
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)
