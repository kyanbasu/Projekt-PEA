#include "../include/Config.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

Config loadConfig(const std::string& filename) {
    Config cfg;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "BLAD: Nie mozna otworzyc pliku konfiguracyjnego " << filename << "!\n";
        exit(1);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string val = line.substr(delimiterPos + 1);
            
            if (key == "data_folder") cfg.data_folder = val;
            else if (key == "output_file") cfg.output_file = val;
            else if (key == "show_progress") cfg.show_progress = std::stoi(val);
            else if (key == "repeats") cfg.repeats = std::stoi(val);
            else if (key == "rand_local_repeats") cfg.rand_local_repeats = std::stoi(val);
        }
    }
    
    // Szukanie plikow .tsp oraz .atsp
    if (fs::exists(cfg.data_folder) && fs::is_directory(cfg.data_folder)) {
        for (const auto& entry : fs::directory_iterator(cfg.data_folder)) {
            std::string ext = entry.path().extension().string();
            // Ignorujemy pliki .opt.tour
            if (ext == ".tsp" || ext == ".atsp") {
                cfg.instances.push_back(entry.path().filename().string());
            }
        }
    } else {
        std::cerr << "BLAD: Folder z danymi nie istnieje: " << cfg.data_folder << "\n";
        exit(1);
    }
    
    return cfg;
}