//
// Created by intak on 26.10.2025.
//

#ifndef MUSIC_PLAYER_PLAYER_H
#define MUSIC_PLAYER_PLAYER_H

#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

class Player {
public:
    Player();
    ~Player();

    bool play(const std::string& path); // начать воспроизведение
    void pause();                       // пауза/возобновление
    void stop();                        // остановка
    bool isPlaying() const;             // играет ли сейчас

private:
    void playbackThread(std::string path); // поток для фонового воспроизведения

    sf::Music music;
    std::thread worker;
    mutable std::mutex mtx;
    std::atomic<bool> running{false};
};

#endif //MUSIC_PLAYER_PLAYER_H
