cmake_minimum_required(VERSION 3.10)
project(RtspServer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 指定源文件目录
file(GLOB SOURCE_CORE "src/*.cpp")
file(GLOB SOURCE_LIVE "src/live/*.cpp")
file(GLOB SOURCE_SCHEDULER "src/scheduler/*.cpp")
set(SOURCES
    ${SOURCE_CORE}
    ${SOURCE_LIVE}
    ${SOURCE_SCHEDULER}
)

# 指定头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/base)
include_directories(${PROJECT_SOURCE_DIR}/include/live)
include_directories(${PROJECT_SOURCE_DIR}/include/scheduler)

# 生成可执行文件
add_executable(RtspServer ${SOURCES})

# 链接库文件
target_link_libraries(RtspServer 
    pthread
)
