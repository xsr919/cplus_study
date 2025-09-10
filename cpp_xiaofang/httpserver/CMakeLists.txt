cmake_minimum_required(VERSION 3.10)
project(HttpServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(httpserver 
    main.cpp 
    HttpServer.cpp 
)

target_link_libraries(httpserver
    Threads::Threads
)

target_compile_options(httpserver PRIVATE
    -Wall
    -Wextra 
    -Wpedantic 
)