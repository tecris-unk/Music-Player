#include "Player.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

bool Player::load(const std::string& path) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!fs::exists(path) || fs::file_size(path) == 0) {
        std::cerr << "Player::load - file missing or empty: " << path << "\n";
        return false;
    }
    if (!music.openFromFile(path)) {
        std::cerr << "Player::load - openFromFile failed: " << path << "\n";
        return false;
    }
    currentPath = path;
    paused = false;
    return true;
}

void Player::play() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!currentPath.empty()) {
        music.play();
        paused = false;
    }
}

void Player::pause() {
    std::lock_guard<std::mutex> lock(mtx);
    auto status = music.getStatus();
    if (status == sf::SoundSource::Status::Playing) {
        music.pause();
        paused = true;
    } else if (status == sf::SoundSource::Status::Paused) {
        music.play();
        paused = false;
    }
}

void Player::stop() {
    std::lock_guard<std::mutex> lock(mtx);
    music.stop();
    paused = false;
}

bool Player::isPlaying() const {
    std::lock_guard<std::mutex> lock(mtx);
    return music.getStatus() == sf::SoundSource::Status::Playing;
}
