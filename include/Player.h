#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <mutex>

class Player {
public:
    Player() = default;
    ~Player() = default;

    // load path into music (open stream). returns true on success
    bool load(const std::string& path);

    // play currently loaded track
    void play();

    // pause/resume toggle
    void pause();

    // stop playback
    void stop();

    bool isPlaying() const;

private:
    sf::Music music;
    std::string currentPath;
    mutable std::mutex mtx;
    bool paused{false};
};
