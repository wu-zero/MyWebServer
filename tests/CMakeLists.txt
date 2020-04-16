project(Test)

file(GLOB_RECURSE testfiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
foreach(file IN LISTS testfiles)
    get_filename_component(test_name ${file} NAME_WE)
#    message(STATUS "${test_name}")
#    message(STATUS "${file}")
    add_executable(${test_name} ${file}) # 添加可执行
    target_link_libraries(${test_name} TcpServer) # 链接
endforeach()
