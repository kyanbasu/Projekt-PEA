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
    int upper_bound_method; // (0: INF, 1: RAND, 2: NN, 3: RNN)
    int time_limit_min; // limit czasu podany w minutach dla danej instancji
    int memory_limit_mb; // limit RAM w megabajtach
    std::vector<std::string> instances;
    int test_type;
};

Config loadConfig(const std::string &filename);
std::vector<std::vector<int>> loadMatrix(const std::string& filepath, int& size);
