#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Clipboard.hpp>
#include "Downloader.h"
#include "Player.h"
#include "TrackList.h"
#include <iostream>
#include <optional>
#include <thread>

int main() {
    sf::RenderWindow window(sf::VideoMode({1200u, 900u}), "Music Downloader");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("assets/Roboto-Regular.ttf")) {
        std::cerr << "Failed to load font\n";
        return 1;
    }

    sf::Text title(font, "Music Downloader", 32);
    title.setPosition({500.f, 20.f});

    sf::RectangleShape urlBox({1000.f, 40.f});
    urlBox.setPosition({120.f, 80.f});
    urlBox.setFillColor(sf::Color(245, 245, 245));
    urlBox.setOutlineColor(sf::Color(180, 180, 180));
    urlBox.setOutlineThickness(2.f);

    sf::Text urlText(font, "", 18);
    urlText.setPosition({130.f, 85.f});
    urlText.setFillColor(sf::Color::Black);

    sf::RectangleShape downloadBtn({180.f, 45.f});
    downloadBtn.setPosition({120.f, 140.f});
    downloadBtn.setFillColor(sf::Color(100, 200, 100));
    sf::Text downloadTxt(font, "Download", 20);
    downloadTxt.setPosition({165.f, 150.f});

    sf::RectangleShape playBtn({180.f, 45.f});
    playBtn.setPosition({330.f, 140.f});
    playBtn.setFillColor(sf::Color(100, 150, 230));
    sf::Text playTxt(font, "Play", 20);
    playTxt.setPosition({395.f, 150.f});

    sf::RectangleShape pauseBtn({180.f, 45.f});
    pauseBtn.setPosition({540.f, 140.f});
    pauseBtn.setFillColor(sf::Color(200, 180, 50));
    sf::Text pauseTxt(font, "Pause", 20);
    pauseTxt.setPosition({595.f, 150.f});

    sf::Text status(font, "Ready", 18);
    status.setPosition({120.f, 200});
    status.setFillColor(sf::Color::White);

    TrackList list;
    Player player;

    bool typing = false;
    std::string input;
    const size_t MAX_INPUT = 200;

    while (window.isOpen()) {
        std::optional<sf::Event> ev;
        while ((ev = window.pollEvent())) {
            auto& event = *ev;

            if (event.is<sf::Event::Closed>()) {
                window.close();
                break;
            }

            if (auto mb = event.getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mp = window.mapPixelToCoords(mb->position);
                typing = urlBox.getGlobalBounds().contains(mp);

                if (downloadBtn.getGlobalBounds().contains(mp)) {
                    if (!input.empty()) {
                        std::string err;
                        std::string fname = filenameFromUrl(input);
                        std::string path = "downloads/" + fname;
                        status.setString("Downloading...");
                        std::thread([&player, &input, path, &list, &status]() {
                            std::string err2;
                            if (downloadFile(input, path, err2)) {
                                list.add(path);
                                status.setString("Downloaded: " + path);
                            } else {
                                status.setString("Download error: " + err2);
                            }
                        }).detach();
                    } else {
                        status.setString("Enter URL");
                    }
                }

                if (playBtn.getGlobalBounds().contains(mp)) {
                    if (list.count > 0) {
                        status.setString("Playing...");
                        std::thread([&player, &list, &status]() {
                            player.play(list.tracks[0]);
                        }).detach();
                    } else {
                        status.setString("No track");
                    }
                }

                if (pauseBtn.getGlobalBounds().contains(mp)) {
                    player.pause();
                    status.setString("Paused");
                }
            }

            if (auto te = event.getIf<sf::Event::TextEntered>()) {
                uint32_t ch = te->unicode;
                if (typing && ch >= 32 && ch < 127 && input.size() < MAX_INPUT) {
                    input.push_back(static_cast<char>(ch));
                    urlText.setString(input);
                }
                if (typing && ch == 8 && !input.empty()) {
                    input.pop_back();
                    urlText.setString(input);
                }
            }

            if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::V && kp->control) {
                    std::string clip = sf::Clipboard::getString().toAnsiString();
                    if (input.size() + clip.size() > MAX_INPUT)
                        clip.resize(MAX_INPUT - input.size());
                    input += clip;
                    urlText.setString(input);
                }
            }
        }

        window.clear(sf::Color(40, 40, 50));
        window.draw(title);
        window.draw(urlBox);
        window.draw(urlText);
        window.draw(downloadBtn);
        window.draw(downloadTxt);
        window.draw(playBtn);
        window.draw(playTxt);
        window.draw(pauseBtn);
        window.draw(pauseTxt);
        window.draw(status);
        window.display();
    }

    return 0;
}
