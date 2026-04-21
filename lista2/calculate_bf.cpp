#include "include/Config.hpp"
#include "include/Utils.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <limits>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

int calculateCost(const vector<int> &path, const vector<vector<int>> &matrix) {
  int cost = 0;
  for (size_t i = 0; i < path.size() - 1; ++i) {
    if(matrix[path[i]][path[i + 1]] == -1) return numeric_limits<int>::max();
    cost += matrix[path[i]][path[i + 1]];
  }
  if(matrix[path.back()][path[0]] == -1) return numeric_limits<int>::max();
  cost += matrix[path.back()][path[0]];
  return cost;
}

int bruteForce(const vector<vector<int>>& matrix) {
    int n = matrix.size();
    vector<int> path(n);
    iota(path.begin(), path.end(), 0);
    
    int best_cost = numeric_limits<int>::max();
    
    do {
        int cost = calculateCost(path, matrix);
        if (cost < best_cost) {
            best_cost = cost;
        }
    } while (next_permutation(path.begin() + 1, path.end()));
    
    return best_cost;
}

int main() {
    string data_folder = "./data_generated/";
    string output_file = "results_bf.csv";
    
    vector<string> instances;
    if (fs::exists(data_folder) && fs::is_directory(data_folder)) {
        for (const auto &entry : fs::directory_iterator(data_folder)) {
            string ext = entry.path().extension().string();
            if (ext == ".tsp" || ext == ".atsp") {
                instances.push_back(entry.path().filename().string());
            }
        }
    }
    
    sort(instances.begin(), instances.end());
    
    ofstream csvOut(output_file);
    csvOut << "Instance,Size,OptimalCost\n";
    
    for(const auto& inst_name : instances) {
        int size = 0;
        string filepath = data_folder + inst_name;
        vector<vector<int>> matrix = loadMatrix(filepath, size);
        
        if (size > 14) {
            cout << "Pominiecie instancji " << inst_name << " (rozmiar " << size << " > 14)\n";
            continue;
        }
        
        cout << "Obliczanie Brute Force dla " << inst_name << " (rozmiar: " << size << ")...\n" << flush;
        int opt_cost = bruteForce(matrix);
        cout << "Znaleziony koszt: " << opt_cost << "\n";
        
        csvOut << inst_name << "," << size << "," << opt_cost << "\n";
        csvOut.flush();
    }
    
    csvOut.close();
    cout << "Zapisano wyniki do " << output_file << "\n";
    return 0;
}
