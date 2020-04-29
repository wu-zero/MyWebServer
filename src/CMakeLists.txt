# 头文件
include_directories(header)
# 源文件
aux_source_directory(source dir_srcs)

find_package(Threads)

add_library(TcpServer ${dir_srcs})
target_link_libraries(TcpServer ${CMAKE_THREAD_LIBS_INIT} )