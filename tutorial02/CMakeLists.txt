cmake_minimum_required (VERSION 2.8)  # CMAKE最低版本号要求
project (tinyjson_test)  # 项目信息

add_library(tinyjson tinyjson.cpp)  # 将 tinyjson.cpp 添加到名为 tinyjson 的库中
add_executable(tinyjson_test test.cpp)  # 生成指定目标
target_link_libraries(tinyjson_test tinyjson)  # 连接名为 tinyjson 的链接库
