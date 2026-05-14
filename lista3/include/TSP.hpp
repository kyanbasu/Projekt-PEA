#pragma once
#include <vector>
#include <utility>

int calculateCost(const std::vector<int>& path, const std::vector<std::vector<int>>& matrix);

// Heurystyki do generowania rozwiazania poczatkowego
int randomSearch(const std::vector<std::vector<int>>& matrix, int local_repeats);
int nearestNeighbour(const std::vector<std::vector<int>>& matrix);
int repetitiveNearestNeighbour(const std::vector<std::vector<int>>& matrix);

// Generowanie rozwiazania poczatkowego
std::vector<int> generateRandomPath(int size);
std::vector<int> nearestNeighbourPath(const std::vector<std::vector<int>>& matrix);
std::vector<int> repetitiveNearestNeighbourPath(const std::vector<std::vector<int>>& matrix);

// Operacje sasiedztwa
std::vector<int> swapNeighbour(const std::vector<int>& path);
std::vector<int> invertNeighbour(const std::vector<int>& path);
std::vector<int> insertNeighbour(const std::vector<int>& path);

// Symulowane wyzarzanie (Simulated Annealing)
// Schematy chlodzenia: 0=geometryczny, 1=liniowy, 2=logarytmiczny
struct SAResult {
    std::vector<int> best_path;
    int best_cost;
    long mem_kb;
    std::vector<double> cost_history; // do analizy zbieznosci
};

SAResult simulatedAnnealing(const std::vector<std::vector<int>>& matrix,
                            double initial_temp, double final_temp,
                            double cooling_rate, int iter_per_temp,
                            int neighbourhood, int init_method,
                            int time_limit_min, int cooling_schedule = 0);
