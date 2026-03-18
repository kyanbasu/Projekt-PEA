#pragma once
#include <string>
#include <vector>

struct Node {
    int id;
    double x;
    double y;
};

struct Config
{
    std::string data_folder;
    std::string output_file;
    bool show_progress;
    int repeats;
    int rand_local_repeats; // ile losowych sciezek wygenerowac dla jednej instancji
    std::vector<std::string> instances;
    int test_type;
};

Config loadConfig(const std::string &filename);
std::vector<std::vector<int>> loadMatrix(const std::string& filepath, int& size);