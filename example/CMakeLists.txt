set(CMAKE_BUILD_TYPE Debug)

INCLUDE_DIRECTORIES( "../" , "../base/" , "../net/" , "./" )
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/../net/)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/../base/)




add_executable(simple_echo echo/echo.cc echo/main.cc)
target_link_libraries(simple_echo leaf_net leaf_base pthread)

add_executable(simple_discard discard/discard.cc discard/main.cc)
target_link_libraries(simple_discard leaf_net leaf_base pthread)

add_subdirectory(curl)
