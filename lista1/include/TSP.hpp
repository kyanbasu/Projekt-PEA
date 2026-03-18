#pragma once
#include <vector>


int calculateCost(const std::vector<int>& path, const std::vector<std::vector<int>>& matrix);
int bruteForce(const std::vector<std::vector<int>>& matrix);
int randomSearch(const std::vector<std::vector<int>>& matrix, int local_repeats);
int nearestNeighbour(const std::vector<std::vector<int>>& matrix);
int repetitiveNearestNeighbour(const std::vector<std::vector<int>>& matrix);