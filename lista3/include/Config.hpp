#pragma once
#include <string>
#include <vector>

struct Node {
    int id;
    double x;
    double y;
};

struct SAConfig {
    std::vector<double> cooling_rates;
    std::vector<double> initial_temps;
    std::vector<double> final_temps;
    std::vector<int> iter_per_temps;
    std::vector<int> neighbourhoods; // 1=swap, 2=invert, 3=insert
    std::vector<int> init_methods;   // 0=random, 1=NN, 2=RNN
};

struct Config
{
    std::string data_folder;
    std::string output_file;
    bool show_progress;
    int repeats;

    // SA parameter sets (comma-separated in config.ini)
    SAConfig sa;

    // Time limit in minutes for a single instance
    int time_limit_min;

    std::vector<std::string> instances;
};

Config loadConfig(const std::string &filename);
std::vector<std::vector<int>> loadMatrix(const std::string& filepath, int& size);

// Helper to split comma-separated strings
std::vector<double> parseDoubleList(const std::string& val);
std::vector<int> parseIntList(const std::string& val);