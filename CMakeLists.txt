cmake_minimum_required(VERSION 3.20)
project(SmtpClient)

set(CMAKE_CXX_STANDARD 17)

add_executable(SmtpClient main.cpp)

target_link_libraries(SmtpClient ws2_32)