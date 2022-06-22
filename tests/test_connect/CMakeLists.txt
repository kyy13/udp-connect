# udp-connect
# Kyle J Burgess

add_executable(
    test_connect
    src/main.cpp
)

target_include_directories(
    test_connect
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

IF (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(
        test_connect
        PRIVATE
        -Wall
        -g
    )
ELSE()
    target_compile_options(
        test_connect
        PRIVATE
        -O3
    )
ENDIF()

target_link_libraries(
    test_connect
    ${PROJECT_NAME}
    -Wl,-allow-multiple-definition
)

add_test(
    NAME
    test_connect
    COMMAND
    test_connect
)

set_target_properties(
    test_connect
    PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
)
