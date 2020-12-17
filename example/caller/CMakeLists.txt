#设置编译文件列表
set(SRC_LIST callUserService.cc ../user.pb.cc)

#设置编译选项，最终生成可执行文件
add_executable(consume ${SRC_LIST})

#设置链接选项
target_link_libraries(consume mrpc protobuf)