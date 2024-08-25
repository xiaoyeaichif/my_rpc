set(SRC_LIST userservice.cc ../user.pb.cc)

add_executable(provider ${SRC_LIST}) # 服务提供者

target_link_libraries(provider mprpc protobuf)