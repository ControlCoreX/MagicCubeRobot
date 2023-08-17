cmake_minimum_required(VERSION 3.0)

project(TUCUBE_PROJECT)

include_directories(. ./tucube ./tucomm)

link_directories(/usr/local/lib)

add_executable(tucube main.cpp ./tucube/tucube.cpp ./tucube/debug.cpp ./tucomm/tucomm.c)

target_link_libraries(tucube wiringPi pthread opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs opencv_videoio opencv_video)