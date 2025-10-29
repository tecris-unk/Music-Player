#include "TrackList.h"
#include <filesystem>


namespace fs = std::filesystem;

static bool hasAudioExt(const std::string& s) {
    if (s.size() < 4) return false;
    std::string ext;
    size_t p = s.find_last_of('.');
    if (p == std::string::npos) return false;
    ext = s.substr(p);

    for (auto &c : ext) if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
    return (ext == ".ogg" || ext == ".wav" || ext == ".flac");
}

void TrackList::scanFolder(const std::string& folderPath) {
    count = 0;
    try {
        if (!fs::exists(folderPath)) return;

        for (auto& e : fs::directory_iterator(folderPath)) {
            if (!e.is_regular_file()) continue;
            std::string p = e.path().string();
            if (hasAudioExt(p)) {
                if (count < 50) tracks[count++] = p;
                else break;
            }
        }
    } catch (...) {
        count = 0;
    }
}

std::string &TrackList::getAt(int idx) {
    return tracks[idx];
}
TrackList::TrackList() = default;
TrackList::~TrackList() = default;