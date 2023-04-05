cmake_minimum_required(VERSION 3.0)

project(imgui)

file(GLOB IMGUI_SOURCES *.cpp)

add_library(imgui STATIC ${IMGUI_SOURCES})