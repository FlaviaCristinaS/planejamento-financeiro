cmake_minimum_required(VERSION 3.30)
project(planejamento-financeiro C)

set(CMAKE_C_STANDARD 11)

add_executable(planejamento-financeiro plan-fin.c)
target_link_libraries(planejamento-financeiro m)