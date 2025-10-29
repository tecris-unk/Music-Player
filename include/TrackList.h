//
// Created by intak on 26.10.2025.
//

#ifndef MUSIC_PLAYER_TRACKLIST_H
#define MUSIC_PLAYER_TRACKLIST_H

#include <string>

struct TrackList {
private:
    std::string tracks[50]; // full paths
public:
    int count = 0;

    void scanFolder(const std::string& folderPath);

    std::string& getAt(int idx);

    TrackList();
    ~TrackList();

};

#endif //MUSIC_PLAYER_TRACKLIST_H
