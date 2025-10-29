//
// Created by intak on 26.10.2025.
//

#ifndef MUSIC_PLAYER_DOWNLOADER_H
#define MUSIC_PLAYER_DOWNLOADER_H
#include <string>

bool downloadFile(const std::string& url, const std::string& path, std::string& error);
std::string filenameFromUrl(const std::string& url);

#endif //MUSIC_PLAYER_DOWNLOADER_H
