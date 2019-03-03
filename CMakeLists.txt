cmake_minimum_required(VERSION 3.13)
project(4CHAN_DOWNLOADER)

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/include/libxml2)
include_directories(/usr/include/libxml++-3.0)
include_directories(/usr/lib/libxml++-3.0/include)

include_directories(/usr/include/glibmm-2.4)
include_directories(/usr/lib/glibmm-2.4/include)

include_directories(/usr/include/glib-2.0)
include_directories(/usr/lib/glib-2.0/include)

set(CMAKE_CXX_FLAGS "-ggdb3 -O0 -lxml++-3.0 -lxml2 -lglibmm-2.4 -lgobject-2.0 -lglib-2.0 -lsigc-2.0 -lcurl")

add_executable(4CHAN_DOWNLOADER
        src/main.cpp
        README.md)
