#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <filesystem>
#include <cstdlib>

#define SWAY_CONFIG_FILE_PATH "/.config/sway/config"
#define CURRENT_DIR_PATH "."
#define BACKGROUND_REGEX R"(^output\s\*\sbg\s)"
#define IMAGE_REGEX R"(\.(jpg|png|jpeg)$)"

namespace utility {
    std::string make_wallpaper_line(const std::string& path) {
        return "output * bg " + path + " fill";
    }

    std::string make_image_path(const std::string& image_filename) {
        return std::filesystem::current_path().string() + '/' + image_filename;
    }
}

namespace check {
    unsigned int err_count = 0;
    std::vector<std::string> err_messages;

    #define PUSH_ERR(message) err_messages.push_back(message); ++err_count;

    void file_path(const std::string& path, const std::string& message_on_fail) {
        if (!std::filesystem::exists(path)) {
            PUSH_ERR(message_on_fail)
        }
    }

    void wallpaper_line_index(int i) {
        if (i == -1) {
            PUSH_ERR("Wallpaper config line haven't been found")
        }
    }

    void image_filename(const std::string& name) {
        std::regex name_pattern(IMAGE_REGEX);
        if (!std::regex_search(name, name_pattern)) {
            PUSH_ERR("Specified file is NOT an image. (Supported formats: png, jpg, jpeg)");
        }
    }

    void images(const std::unordered_map<int, std::string>& map) {
        if (map.empty()) {
            PUSH_ERR("No images found in the current dir")
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
    const std::string home = std::getenv("HOME");
    const std::string sway_config_path = home + SWAY_CONFIG_FILE_PATH;

    check::file_path(sway_config_path, "Sway config file doesn't exist!");
    CHECK_RESULT()

    std::string image_path;

    if (argc == 1) {
        // No any image's path specified, trying to check current user location for images
        std::regex image_pattern(IMAGE_REGEX);
        std::unordered_map<int, std::string> found_images;

        int index = 1;
        for (const auto& file : std::filesystem::directory_iterator(CURRENT_DIR_PATH)) {
            const std::string file_name = file.path().filename().string();

            if (std::regex_search(file_name, image_pattern)) {
                found_images[index] = file_name;
                std::cout << index << ": " + file_name << '\n';
                ++index;
            }
        }

        check::images(found_images);
        CHECK_RESULT()
        
        std::cout << "0: Cancel" << '\n';

    choose_image:
        std::cout << '\n';
        std::cout << "Which image set on wall? (number):" << '\n';
        std::cin >> index;

        if (index == 0) {
            std::cout << "Canceled..." << '\n';
            return EXIT_FAILURE;
        }

        if (!found_images.contains(index)) {
            std::cout << "Specified number is NOT valid" << '\n';
            goto choose_image;
        }

        image_path = utility::make_image_path(found_images.at(index));
    }
    else if (argc >= 2) {
        // Image name specified
        check::image_filename(argv[1]);
        CHECK_RESULT()

        image_path = utility::make_image_path(argv[1]);
    }

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
    for (size_t i = 0; i < sway_config_lines.size(); ++i) {
        if (std::regex_search(sway_config_lines[i], bg_pattern)) {
            wallpaper_line = i;
            break;
        }
    }
    
    check::wallpaper_line_index(wallpaper_line);
    
    CHECK_RESULT()

    temp_line = utility::make_wallpaper_line(image_path);
    
    if (temp_line == sway_config_lines[wallpaper_line]) {
        std::cout << "Wallpaper is the same. Nothing has been changed" << '\n';
        return EXIT_FAILURE;
    }

    sway_config_lines[wallpaper_line] = temp_line;

    // Backup original sway config file
    std::filesystem::rename(sway_config_path, sway_config_path + ".back");

    std::ofstream out_sway_config(sway_config_path);
    for (const auto& line : sway_config_lines) {
        out_sway_config << line << '\n';
    }

    std::cout << "Wallpaper has been updated in sway's config file. Please reload sway. (command: swaymsg reload)" << '\n';

    return EXIT_SUCCESS;
}
