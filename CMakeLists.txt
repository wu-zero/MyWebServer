cmake_minimum_required(VERSION 3.10)
project(MyWebServer)
set(CMAKE_CXX_STANDARD 11)

## 头文件
include_directories(src/header)

# 可执行文件
add_executable(MyWebServer main.cpp  HttpHandler.cpp HttpHandler.h HttpConnectionManager.h HttpConnectionManager.cpp)
# 链接
target_link_libraries(MyWebServer TcpServer)

# src子目录
add_subdirectory(src)
# tests子目录
add_subdirectory(tests)