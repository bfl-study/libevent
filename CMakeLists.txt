cmake_minimum_required(VERSION 2.8.0)

project(libevent_study)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(event_http_src event_http.c)
add_executable(event_http ${event_http_src})
target_link_libraries(event_http event)

set(event_server_src event_server.c)
add_executable(event_server ${event_server_src})
target_link_libraries(event_server event)

set(event_client_src event_client.c)
add_executable(event_client ${event_client_src})
target_link_libraries(event_client pthread)

set(hello_world_server_src hello_world_server.c)
add_executable(hello_world_server ${hello_world_server_src})
target_link_libraries(hello_world_server event)

