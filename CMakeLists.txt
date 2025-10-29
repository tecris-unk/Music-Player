cmake_minimum_required(VERSION 3.29)

project(music_player)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3.0.2 COMPONENTS Window Audio Graphics REQUIRED)
find_package(CURL REQUIRED)



add_executable(music_player
        src/main.cpp
        src/Downloader.cpp
        include/Downloader.h
        src/TrackList.cpp
        include/TrackList.h
        src/Player.cpp
        include/Player.h
)
target_include_directories(music_player PRIVATE include)
target_link_libraries(music_player PRIVATE
        SFML::Window
        SFML::Audio
        SFML::Graphics
        CURL::libcurl
)
