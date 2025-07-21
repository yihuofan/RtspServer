cmake_minimum_required(VERSION 3.10)
project(RtspServer)

# 设置C++标准和编译选项
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# 包含头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/net
    ${CMAKE_CURRENT_SOURCE_DIR}/src/xop
    ${CMAKE_CURRENT_SOURCE_DIR}/src/3rdpart
)

# 收集源码文件
file(GLOB NET_SRCS src/net/*.cpp)
file(GLOB XOP_SRCS src/xop/*.cpp)

# 生成rtsp_server和rtsp_pusher两个可执行文件
add_executable(rtsp_server ${NET_SRCS} ${XOP_SRCS} example/rtsp_server.cpp)
add_executable(rtsp_pusher ${NET_SRCS} ${XOP_SRCS} example/rtsp_pusher.cpp)

# 链接系统库
target_link_libraries(rtsp_server rt pthread dl m)
target_link_libraries(rtsp_pusher rt pthread dl m)

# 添加清理目标
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "rtsp_server;rtsp_pusher")