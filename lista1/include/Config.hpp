#pragma once
#include <string>
#include <vector>

struct Config
{
    std::string data_folder;
    std::string output_file;
    bool show_progress;
    int repeats;
    int rand_local_repeats; // ile losowych sciezek wygenerowac dla jednej instancji
    std::vector<std::string> instances;
};

Config loadConfig(const std::string &filename);