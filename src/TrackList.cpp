//
// Created by intak on 26.10.2025.
//

#include "TrackList.h"
#include <iostream>

bool TrackList::add(const std::string& path){
    if(count >= 50){
        std::cout << "you daun" << std::endl;
        return false;
    }
    tracks[count++] = path;
    return true;
}

void TrackList::list() const{
    for(int i = 0;i < count; ++i)
        std::cout << i + 2 << ". " << tracks[i] << std::endl;
}
