set(SRC_LIST calluserservice.cc ../user.pb.cc)

add_executable(consumer ${SRC_LIST}) # 服务提供者

target_link_libraries(consumer mprpc protobuf)