#include "Player.h"
#include <iostream>
#include <filesystem>

Player::Player() {}
Player::~Player() {
    stop(); // гарантированно остановим поток при выходе
    if (worker.joinable()) worker.join();
}

bool Player::play(const std::string& path) {
    stop(); // если уже что-то играет, остановим старое

    // Проверяем, что файл существует
    if (!std::filesystem::exists(path)) {
        std::cerr << "❌ Ошибка: файл не найден: " << path << std::endl;
        return false;
    }

    running = true;
    worker = std::thread(&Player::playbackThread, this, path);
    return true;
}

void Player::playbackThread(std::string path) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // диск успевает закрыть файл

    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!music.openFromFile(path)) {
            std::cerr << "❌ Ошибка: не удалось открыть " << path << std::endl;
            running = false;
            return;
        }
        music.play();
    }

    // ждём окончания воспроизведения
    while (running) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (music.getStatus() == sf::SoundSource::Status::Stopped)
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::lock_guard<std::mutex> lock(mtx);
    music.stop();
    running = false;
}

void Player::pause() {
    std::lock_guard<std::mutex> lock(mtx);
    auto status = music.getStatus();
    if (status == sf::SoundSource::Status::Playing) {
        music.pause();
        std::cout << "⏸ Пауза\n";
    } else if (status == sf::SoundSource::Status::Paused) {
        music.play();
        std::cout << "▶ Продолжение\n";
    }
}

void Player::stop() {
    running = false;
    {
        std::lock_guard<std::mutex> lock(mtx);
        music.stop();
    }
    if (worker.joinable()) worker.join();
}

bool Player::isPlaying() const {
    std::lock_guard<std::mutex> lock(mtx);
    return music.getStatus() == sf::SoundSource::Status::Playing;
}
