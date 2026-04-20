#pragma once
#include <vector>

int calculateCost(const std::vector<int>& path, const std::vector<std::vector<int>>& matrix);

// Heurystyki uzywane do obslugi Upper Bound (górnego ograniczenia)
int randomSearch(const std::vector<std::vector<int>>& matrix, int local_repeats);
int nearestNeighbour(const std::vector<std::vector<int>>& matrix);
int repetitiveNearestNeighbour(const std::vector<std::vector<int>>& matrix);

// Metody Branch and Bound dla Listy 2
int branchAndBoundBFS(const std::vector<std::vector<int>>& matrix, int initial_upper_bound, int time_limit_min);
int branchAndBoundDFS(const std::vector<std::vector<int>>& matrix, int initial_upper_bound, int time_limit_min);
int branchAndBoundLC(const std::vector<std::vector<int>>& matrix, int initial_upper_bound, int time_limit_min);
