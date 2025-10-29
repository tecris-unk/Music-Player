//
// Created by intak on 26.10.2025.
//

#ifndef MUSIC_PLAYER_TRACKLIST_H
#define MUSIC_PLAYER_TRACKLIST_H
#pragma once
#include <string>

struct TrackList{
    std::string tracks[50];
    int count = 0;

    bool add(const std::string& path);
    void list() const;
};
#endif //MUSIC_PLAYER_TRACKLIST_H
