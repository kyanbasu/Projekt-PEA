#pragma once
#include <vector>

int calculateCost(const std::vector<int>& path, const std::vector<std::vector<int>>& matrix);
int calculateMST(const std::vector<std::vector<int>>& matrix);

// Heurystyki do generowania rozwiazania poczatkowego
int randomSearch(const std::vector<std::vector<int>>& matrix, int local_repeats);
int nearestNeighbour(const std::vector<std::vector<int>>& matrix);
int repetitiveNearestNeighbour(const std::vector<std::vector<int>>& matrix);

// Generowanie rozwiazania poczatkowego
std::vector<int> generateRandomPath(int size);
std::vector<int> nearestNeighbourPath(const std::vector<std::vector<int>>& matrix);
std::vector<int> repetitiveNearestNeighbourPath(const std::vector<std::vector<int>>& matrix);

// Algorytm Mrowkowy (Ant Colony System)
struct ACOResult {
    std::vector<int> best_path;
    int best_cost;
    long mem_kb;
    std::vector<double> cost_history; // do analizy zbieznosci
    int lb; // Lower Bound z MST
};

ACOResult antColonyOptimization(const std::vector<std::vector<int>>& matrix,
                                double alpha, double beta, double evaporation_rate,
                                int ants_count, int iterations,
                                int init_method, int aco_variant, int time_limit_min,
                                bool is_symmetric);
