#include <SFML/Graphics.hpp>
#include <SFML/Window/Clipboard.hpp> // SFML 3
#include "Downloader.h"
#include "Player.h"
#include "TrackList.h"
#include <optional>
#include <thread>
#include <atomic>
#include <iostream>
#include <filesystem>
#include <vector>
#include <windows.h>
namespace fs = std::filesystem;

int main() {

    // window larger to fit long links
    sf::RenderWindow window(sf::VideoMode({1000u, 700u}), "MyMusicPlayer (SFML 3.0.2)");
    window.setFramerateLimit(60);

    // load font
    sf::Font font;
    if (!font.openFromFile("assets/Roboto-Regular.ttf")) {
        std::cerr << "Failed to load font 'assets/Roboto-Regular.ttf'\n";
        return 1;
    }

    // UI elements
    sf::Text title(font, "Music Downloader", 34);
    title.setPosition({20.f, 12.f});
    title.setFillColor(sf::Color::White);

    sf::RectangleShape urlBox({820.f, 44.f});
    urlBox.setPosition({20.f, 70.f});
    urlBox.setFillColor(sf::Color(245,245,245));
    urlBox.setOutlineThickness(2.f);
    urlBox.setOutlineColor({160,160,160});

    sf::Text urlText(font, "", 20);
    urlText.setPosition({28.f, 78.f});
    urlText.setFillColor(sf::Color::Black);

    sf::RectangleShape downloadBtn({140.f, 44.f});
    downloadBtn.setPosition({860.f, 70.f});
    downloadBtn.setFillColor({90,200,100});
    sf::Text downloadLabel(font, "Download", 18);
    downloadLabel.setPosition({882.f, 82.f});
    downloadLabel.setFillColor(sf::Color::Black);

    // Controls
    sf::RectangleShape playBtn({120.f, 40.f});
    playBtn.setPosition({20.f, 540.f});
    playBtn.setFillColor({100,150,230});
    sf::Text playLabel(font, "Play", 18);
    playLabel.setPosition({60.f, 546.f});

    sf::RectangleShape pauseBtn({160.f, 40.f});
    pauseBtn.setPosition({160.f, 540.f});
    pauseBtn.setFillColor({200,180,50});
    sf::Text pauseLabel(font, "Pause/Resume", 18);
    pauseLabel.setPosition({176.f, 546.f});

    sf::RectangleShape stopBtn({120.f, 40.f});
    stopBtn.setPosition({340.f, 540.f});
    stopBtn.setFillColor({200,100,100});
    sf::Text stopLabel(font, "Stop", 18);
    stopLabel.setPosition({380.f, 546.f});

    sf::Text status(font, "Ready", 18);
    status.setPosition({20.f, 600.f});
    status.setFillColor(sf::Color::White);

    sf::Text listTitle(font, "Downloads (click to select):", 20);
    listTitle.setPosition({20.f, 130.f});
    listTitle.setFillColor(sf::Color::White);

    // backend
    TrackList trackList;
    Player player;

    trackList.scanFolder("downloads");
    std::vector <sf::Text> trackTexts;
    auto rebuildTexts = [&]() {
        float y = 160.f;
        for (int i=0;i<50;++i) {
            if (i < trackList.count) {
                fs::path p(trackList.getAt(i));
                std::string name = p.filename().string();
                sf::Text text(font, name);
                trackTexts.push_back(text);
                trackTexts[i].setString(name);
                trackTexts[i].setPosition({28.f, y});
                trackTexts[i].setFillColor(sf::Color::White);
                y += 28.f;
            } else {
                sf::Text text(font, "");
                trackTexts.push_back(text);
            }
        }
    };
    rebuildTexts();

    std::optional<int> selectedIndex;
    bool typing = false;
    std::string input;
    const size_t MAX_INPUT = 512;

    std::atomic<bool> downloading(false);
    std::atomic<bool> needRescan(false);

    while (window.isOpen()) {
        std::optional<sf::Event> ev;
        while ((ev = window.pollEvent())) {
            const auto& event = *ev;
            if (event.is<sf::Event::Closed>()) { window.close(); break; }

            // mouse pressed
            if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mp = window.mapPixelToCoords(mb->position);

                typing = urlBox.getGlobalBounds().contains(mp);

                // download
                if (downloadBtn.getGlobalBounds().contains(mp)) {
                    if (input.empty()) {
                        status.setString("Enter URL (or paste with Ctrl+V).");
                    } else if (downloading.load()) {
                        status.setString("Download in progress...");
                    } else {
                        // determine filename
                        std::string fname = filenameFromUrl(input);
                        if (fname.empty() || fname.find('.') == std::string::npos) {
                            fname = "downloaded_" + std::to_string(std::time(nullptr)) + ".ogg";
                        }
                        std::string outpath = std::string("downloads/") + fname;
                        downloading = true;
                        status.setString("Downloading...");
                        std::thread([input, outpath, &downloading, &needRescan]() {
                            std::string err;
                            bool ok = downloadFile(input, outpath, err);
                            if (!ok) {
                                std::cerr << "Download error: " << err << "\n";
                            } else {
                                std::cout << "Saved: " << outpath << "\n";
                                needRescan = true;
                            }
                            downloading = false;
                        }).detach();
                    }
                }

                // play
                if (playBtn.getGlobalBounds().contains(mp)) {
                    if (selectedIndex.has_value()) {
                        int idx = *selectedIndex;
                        if (idx >= 0 && idx < trackList.count) {
                            std::string path = trackList.getAt(idx);
                            if (player.load(path)) {
                                player.play();
                                status.setString("Playing: " + fs::path(path).filename().string());
                            } else status.setString("Load failed");
                        }
                    } else if (trackList.count > 0) {
                        std::string path = trackList.getAt(0);
                        if (player.load(path)) {
                            player.play();
                            status.setString("Playing: " + fs::path(path).filename().string());
                        } else status.setString("Load failed");
                    } else {
                        status.setString("No tracks");
                    }
                }

                // pause/resume
                if (pauseBtn.getGlobalBounds().contains(mp)) {
                    player.pause();
                    status.setString("Pause/Resume");
                }

                // stop
                if (stopBtn.getGlobalBounds().contains(mp)) {
                    player.stop();
                    status.setString("Stopped");
                }

                // selecting track by clicking text
                for (int i=0; i<trackList.count; ++i) {
                    if (trackTexts[i].getGlobalBounds().contains(mp)) {
                        selectedIndex = i;
                        for (int j=0;j<trackList.count;++j) trackTexts[j].setFillColor(sf::Color::White);
                        trackTexts[i].setFillColor(sf::Color::Yellow);
                        status.setString("Selected: " + trackTexts[i].getString());
                        break;
                    }
                }
            }

            // mouse wheel -> scroll visible texts by moving their Y positions
            if (const auto* mw = event.getIf<sf::Event::MouseWheelScrolled>()) {
                float delta = mw->delta * 28.f;
                for (int i=0;i<50;++i) {
                    sf::Vector2f p = trackTexts[i].getPosition();
                    trackTexts[i].setPosition({p.x, p.y - delta});
                }
            }

            // text entered => url typing
            if (const auto* te = event.getIf<sf::Event::TextEntered>()) {
                uint32_t ch = te->unicode;
                if (typing) {
                    if (ch == 8) { if (!input.empty()) input.pop_back(); }
                    else if (ch >= 32 && ch < 127 && input.size() < MAX_INPUT) input.push_back(static_cast<char>(ch));
                    urlText.setString(input);
                }
            }

            // key pressed -> Ctrl+V and Enter
            if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::V && kp->control && typing) {
                    std::string clip = sf::Clipboard::getString().toAnsiString();
                    if (input.size() + clip.size() > MAX_INPUT) clip.resize(MAX_INPUT - input.size());
                    input += clip;
                    urlText.setString(input);
                }
                if (kp->code == sf::Keyboard::Key::Enter && typing) {
                    // same as download
                    if (!input.empty() && !downloading.load()) {
                        std::string fname = filenameFromUrl(input);
                        if (fname.empty() || fname.find('.') == std::string::npos)
                            fname = "downloaded_" + std::to_string(std::time(nullptr)) + ".ogg";
                        std::string outpath = std::string("downloads/") + fname;
                        downloading = true;
                        status.setString("Downloading...");
                        std::thread([input, outpath, &downloading, &needRescan]() {
                            std::string err;
                            bool ok = downloadFile(input, outpath, err);
                            if (!ok) {
                                std::cerr << "Download error: " << err << "\n";
                            } else {
                                needRescan = true;
                            }
                            downloading = false;
                        }).detach();
                    }
                }
            }
        } // events

        // if a background download finished and signalled, rescan and rebuild texts in main thread
        if (needRescan.exchange(false)) {
            trackList.scanFolder("downloads");
            rebuildTexts();
        }



        // draw
        window.clear({35,35,40});
        window.draw(title);
        window.draw(urlBox);
        window.draw(urlText);
        window.draw(downloadBtn);
        window.draw(downloadLabel);
        window.draw(listTitle);
        window.draw(playBtn);
        window.draw(playLabel);
        window.draw(pauseBtn);
        window.draw(pauseLabel);
        window.draw(stopBtn);
        window.draw(stopLabel);
        window.draw(status);

        // draw tracks
        for (int i=0;i<trackList.count;++i) {
            if (!trackTexts[i].getString().isEmpty()) window.draw(trackTexts[i]);
        }

        window.display();
    }

    return 0;
}
