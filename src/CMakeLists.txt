#设置编译文件列表
#aux_source_directory(. SRC_LIST)
set(SRC_LIST 
    mrpcApplication.cc 
    mrpcChannel.cc 
    mrpcConfig.cc
    mrpcController.cc
    mrpcHeader.pb.cc 
    mrpcLogger.cc
    mrpcProvider.cc
    zookeeperUtil.cc)

#设置编译选项，最终生成静态库文件
add_library(mrpc ${SRC_LIST})

#设置链接选项
target_link_libraries(mrpc muduo_net muduo_base protobuf pthread zookeeper_mt)
