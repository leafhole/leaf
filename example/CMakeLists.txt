INCLUDE_DIRECTORIES("../", "../base/", "../net/")
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/../net/)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/../base/)




add_executable(simple_echo echo/echo.cc echo/main.cc)
target_link_libraries(simple_echo leaf_net leaf_base pthread)
