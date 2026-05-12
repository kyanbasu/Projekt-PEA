#pragma once
#include <vector>

int calculateCost(const std::vector<int>& path, const std::vector<std::vector<int>>& matrix);

// Heurystyki uzywane do obslugi Upper Bound (górnego ograniczenia)
int randomSearch(const std::vector<std::vector<int>>& matrix, int local_repeats);
int nearestNeighbour(const std::vector<std::vector<int>>& matrix);
int repetitiveNearestNeighbour(const std::vector<std::vector<int>>& matrix);

// Metody Branch and Bound dla Listy 2
std::pair<int, long> branchAndBoundBFS(const std::vector<std::vector<int>>& matrix, int initial_upper_bound, int time_limit_min, int memory_limit_mb);
std::pair<int, long> branchAndBoundDFS(const std::vector<std::vector<int>>& matrix, int initial_upper_bound, int time_limit_min, int memory_limit_mb);
std::pair<int, long> branchAndBoundLC(const std::vector<std::vector<int>>& matrix, int initial_upper_bound, int time_limit_min, int memory_limit_mb);
