cmake_minimum_required(VERSION 3.0.0)
project(Proj1 VERSION 0.1.0 LANGUAGES C)

add_library(SI ./src/SI.c ./src/SI.h ./src/utils.c ./src/utils.h)
add_library(UTILS ./src/utils.c ./src/utils.h)
add_library(THREAD ./src/thread1_utils.c ./src/thread1_utils.h)

add_executable(Proj1 ./src/main.c)
target_link_libraries(Proj1 SI)
target_link_libraries(Proj1 UTILS)
target_link_libraries(Proj1 THREAD)
target_link_libraries(Proj1 wiringPi)
target_link_libraries(Proj1 bcm2835)
target_link_libraries(Proj1 pthread)