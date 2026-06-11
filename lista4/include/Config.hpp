#pragma once
#include <string>
#include <vector>

struct Node {
    int id;
    double x;
    double y;
};

struct ACOConfig {
    std::vector<double> alphas;
    std::vector<double> betas;
    std::vector<double> evaporation_rates; // rho
    std::vector<int> ants_counts;
    std::vector<int> iterations;
    std::vector<int> init_methods; // 0=random, 1=NN, 2=RNN (do inicjalizacji feromonu)
    std::vector<int> aco_variants; // 0=AS, 1=MMAS
};

struct Config
{
    std::string data_folder;
    std::string output_file;
    bool show_progress;
    int repeats;

    // ACO parameter sets (comma-separated in config.ini)
    ACOConfig aco;

    // Time limit in minutes for a single instance
    int time_limit_min;

    std::vector<std::string> instances;
};

Config loadConfig(const std::string &filename);
std::vector<std::vector<int>> loadMatrix(const std::string& filepath, int& size);

// Helper to split comma-separated strings
std::vector<double> parseDoubleList(const std::string& val);
std::vector<int> parseIntList(const std::string& val);