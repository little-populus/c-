cmake_minimum_required(VERSION 3.20.0)

project(c++ VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

add_executable(c++ main.cpp)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(c++ X11::X11 X11::Xrender X11::Xcomposite)