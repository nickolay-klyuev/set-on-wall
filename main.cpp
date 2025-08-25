#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <cstdlib>

#define SWAY_CONFIG_FILE_PATH "/.config/sway/config"
#define BACKGROUND_REGEX R"(^output\s\*\sbg\s)"

namespace sway {
    std::string make_wallpaper_line(const std::string& path) {
        return "output * bg " + path + " fill";
    }
}

namespace check {
    unsigned int err_count = 0;
    std::vector<std::string> err_messages;

    void file_path(const std::string& path, const std::string& message_on_fail) {
        if (!std::filesystem::exists(path)) {
            err_messages.push_back(message_on_fail);
            ++err_count;
        }
    }

    void wallpaper_line_index(int i) {
        if (i == -1) {
            err_messages.push_back("Wallpaper config line haven't been found");
            ++err_count;
        }
    }

    void arguments(int argc) {
        if (argc != 2) {
            err_messages.push_back("Programm allows 1 argument with an image's path");
            ++err_count;
        }
    }

    int result() {
        if (err_count > 0) {
            for (const auto& message : err_messages) {
                std::cerr << message << '\n';
            }

            return EXIT_FAILURE;
        }
        
        return EXIT_SUCCESS;
    }
}

#define CHECK_RESULT() if (check::result() == EXIT_FAILURE) { return EXIT_FAILURE; }

int main (int argc, char *argv[]) {

    check::arguments(argc);

    CHECK_RESULT()

    const std::string home = std::getenv("HOME");
    const std::string sway_config_path = home + SWAY_CONFIG_FILE_PATH;
    const std::string image_path = std::filesystem::current_path().string() + '/' + argv[1];

    check::file_path(sway_config_path, "Sway config file doesn't exist!");
    check::file_path(image_path, "Image's path is not valid: " + image_path);

    CHECK_RESULT()

    std::ifstream sway_config_file(sway_config_path);

    std::vector<std::string> sway_config_lines;
    std::string temp_line;

    while (std::getline(sway_config_file, temp_line)) {
        sway_config_lines.push_back(temp_line);
    }

    std::regex bg_pattern(BACKGROUND_REGEX);
    
    int wallpaper_line = -1;
    for (int i = 0; i < sway_config_lines.size(); ++i) {
        if (std::regex_search(sway_config_lines[i], bg_pattern)) {
            wallpaper_line = i;
            break;
        }
    }
    
    check::wallpaper_line_index(wallpaper_line);
    
    CHECK_RESULT()

    temp_line = sway::make_wallpaper_line(image_path);
    
    if (temp_line == sway_config_lines[wallpaper_line]) {
        std::cout << "Wallpaper is the same. Nothing has been changed" << '\n';
        return EXIT_SUCCESS;
    }

    sway_config_lines[wallpaper_line] = temp_line;

    // Backup original sway config file
    std::filesystem::rename(sway_config_path, sway_config_path + ".back");

    std::ofstream out_sway_config(sway_config_path);
    for (const auto& line : sway_config_lines) {
        out_sway_config << line << '\n';
    }

    std::cout << "Wallpaper has been updated" << '\n';

    return EXIT_SUCCESS;
}
