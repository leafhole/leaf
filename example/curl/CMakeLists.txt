add_library(leaf_curl Curl.cc)
target_link_libraries(leaf_curl leaf_net)
target_link_libraries(leaf_curl leaf_base)
target_link_libraries(leaf_curl curl)

add_executable(mcurl mcurl.cc)
target_link_libraries(mcurl leaf_net leaf_base leaf_curl pthread)

add_executable(download download.cc)
target_link_libraries(download leaf_net leaf_base leaf_curl pthread)
