cmake_minimum_required(VERSION 3.10)
project(MyWebServer)
set(CMAKE_CXX_STANDARD 11)

# 头文件
include_directories(src/header)
# 源文件
aux_source_directory(src/source dir_srcs )

# 链接
set(WORK ${dir_srcs})
add_executable(MyWebServer ${WORK} main.cpp MyServer.h MyServer.cpp)
find_package (Threads)
target_link_libraries (MyWebServer)


# client
add_executable(Client ${WORK} MyClient.cpp)