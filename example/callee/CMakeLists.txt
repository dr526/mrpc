#设置编译文件列表
set(SRC_LIST userService.cc ../user.pb.cc)

#设置编译选项，最终生成可执行文件
add_executable(provider ${SRC_LIST})

#设置链接选项
target_link_libraries(provider mrpc protobuf)