# 头文件
include_directories(header)
# 源文件
aux_source_directory(source dir_srcs)

add_library(TcpServer ${dir_srcs})
