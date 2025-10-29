#include "Downloader.h"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

std::string filenameFromUrl(const std::string& url) {
    unsigned long long pos = url.find_last_of("/\\");
    if (pos == std::string::npos)
        return "file.ogg";
    return url.substr(pos + 1);
}

size_t writeData(void* ptr, long long size, long long nmemb, void* userdata) {
    auto file = static_cast<std::ofstream*>(userdata);
    file->write(static_cast<const char*>(ptr), size * nmemb);
    return size * nmemb;
}

bool downloadFile(const std::string& url, const std::string& output, std::string& errorMsg) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        errorMsg = "Cant initialize curl";
        return false;
    }

    std::ofstream file(output, std::ios::binary);
    if (!file.is_open()) {
        errorMsg = "Cant open file";
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SFML-Downloader/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    file.close();
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        errorMsg = curl_easy_strerror(res);
        return false;
    }

    if (!fs::exists(output) || fs::file_size(output) == 0) {
        errorMsg = "File isn't downloaded correctly";
        return false;
    }

    return true;
}
