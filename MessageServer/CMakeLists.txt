cmake_minimum_required(VERSION 2.8.4)
project(MessageServer)

set(SERVER msgd.cc server.cc msg_server.cc)
set(CLIENT msg.cc client.cc msg_client.cc util.cc)
add_executable(MessageServer ${SERVER})
add_executable(MessageClient ${CLIENT})