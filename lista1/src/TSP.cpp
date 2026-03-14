#include "../include/TSP.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>

using namespace std;

int calculateEuc2D(const Node &a, const Node &b) {
  double xd = a.x - b.x;
  double yd = a.y - b.y;
  return static_cast<int>(round(sqrt(xd * xd + yd * yd)));
}

vector<vector<int>> loadMatrix(const string &filepath, int &size) {
  ifstream file(filepath);
  if (!file.is_open()) {
    cerr << "BLAD: Nie znaleziono pliku " << filepath << "\n";
    exit(1);
  }

  string line;
  bool is_euc2d = false;
  bool in_coord_section = false;
  bool in_matrix_section = false;
  vector<Node> nodes;

  size = 0;

  while (getline(file, line)) {
    if (line.empty() || line.find("EOF") != string::npos)
      continue;

    // rozmiar
    if (line.find("DIMENSION") != string::npos) {
      auto colon_pos = line.find(':');
      if (colon_pos != string::npos) {
        size = stoi(line.substr(colon_pos + 1));
      } else {
        stringstream ss(line);
        string dummy;
        ss >> dummy >> size;
      }
      continue;
    }

    // wagi
    if (line.find("EDGE_WEIGHT_TYPE") != string::npos) {
      if (line.find("EUC_2D") != string::npos) {
        is_euc2d = true;
      }
      continue;
    }

    // koordynaty
    if (line.find("NODE_COORD_SECTION") != string::npos) {
      in_coord_section = true;
      in_matrix_section = false;
      continue;
    } else if (line.find("EDGE_WEIGHT_SECTION") != string::npos) {
      in_matrix_section = true;
      in_coord_section = false;
      continue;
    }

    if (in_coord_section) {
      stringstream ss(line);
      Node n;
      if (ss >> n.id >> n.x >> n.y) {
        nodes.push_back(n);
      }
    } else if (in_matrix_section) {
      break;
    }
  }

  vector<vector<int>> matrix(size, vector<int>(size, 0));

  if (is_euc2d && !nodes.empty()) {
    for (int i = 0; i < size; ++i) {
      for (int j = 0; j < size; ++j) {
        if (i == j) {
          matrix[i][j] = 0;
        } else {
          matrix[i][j] = calculateEuc2D(nodes[i], nodes[j]);
        }
      }
    }
  } else {
    for (int i = 0; i < size; ++i) {
      for (int j = 0; j < size; ++j) {
        file >> matrix[i][j];
      }
    }
  }

  return matrix;
}

int calculateCost(const vector<int> &path, const vector<vector<int>> &matrix) {
  int cost = 0;
  for (size_t i = 0; i < path.size() - 1; ++i) {
    cost += matrix[path[i]][path[i + 1]];
  }
  cost += matrix[path.back()][path[0]];
  return cost;
}

// --- Brute Force ---
int bruteForce(const vector<vector<int>> &matrix) {
  int size = matrix.size();
  vector<int> path(size);
  iota(path.begin(), path.end(), 0);

  int min_cost = numeric_limits<int>::max();
  do {
    int current_cost = calculateCost(path, matrix);
    if (current_cost < min_cost)
      min_cost = current_cost;
  } while (next_permutation(path.begin() + 1, path.end()));
  // +1 by zawsze zaczynac z zera (optymalizacja dla symetrycznych i cykli)

  return min_cost;
}

// --- RAND ---
int randomSearch(const vector<vector<int>> &matrix, int local_repeats) {
  int size = matrix.size();
  vector<int> path(size);
  iota(path.begin(), path.end(), 0);

  // pseudolosowe, aby zachowac powtarzalnosc
  static mt19937 g(42);
  int min_cost = numeric_limits<int>::max();

  for (int i = 0; i < local_repeats; ++i) {
    shuffle(path.begin() + 1, path.end(), g);
    int current_cost = calculateCost(path, matrix);
    if (current_cost < min_cost)
      min_cost = current_cost;
  }

  return min_cost;
}

// --- Nearest Neighbour (NN) ---
int nearestNeighbour(const vector<vector<int>> &matrix) {
  int n = matrix.size();
  vector<bool> visited(n, false);
  int current_node = 0; // start sztywno od węzła 0
  visited[current_node] = true;

  int total_cost = 0;

  for (int step = 1; step < n; ++step) {
    int next_node = -1;
    int min_weight = numeric_limits<int>::max();

    for (int i = 0; i < n; ++i) {
      if (!visited[i] && matrix[current_node][i] < min_weight) {
        min_weight = matrix[current_node][i];
        next_node = i;
      }
    }

    visited[next_node] = true;
    total_cost += min_weight;
    current_node = next_node;
  }

  total_cost += matrix[current_node][0]; // Powrot do startu
  return total_cost;
}

// --- Repetitive Nearest Neighbour (RNN) z rozgalezianiem ---
#include <chrono>

void exploreRNN(const vector<vector<int>> &matrix, int start_node,
                int current_node, vector<bool> &visited, int current_cost,
                int visited_count, int &best_overall_cost,
                chrono::time_point<chrono::high_resolution_clock> start_time,
                double timeout_ms, bool &time_exceeded) {

  if (time_exceeded)
    return;

  auto now = chrono::high_resolution_clock::now();
  chrono::duration<double, milli> elapsed = now - start_time;
  if (elapsed.count() > timeout_ms) {
    time_exceeded = true;
    return;
  }

  // pruning do optymalzacji
  // jesli juz teraz jest gorszy lub rowny najlepszemu wynikowi z innej galezi,
  // to koniec
  if (current_cost >= best_overall_cost) {
    return;
  }

  int n = matrix.size();
  if (visited_count == n) {
    int return_edge = matrix[current_node][start_node];
    // -1 to nieskonczonosc dla wygenerowanych macierzy - brak sciezki powrotnej
    if (return_edge != -1) {
      int final_cost = current_cost + return_edge;
      if (final_cost < best_overall_cost)
        best_overall_cost = final_cost;
    }
    return;
  }

  int min_weight = numeric_limits<int>::max();
  for (int i = 0; i < n; ++i) {
    if (!visited[i] && i != current_node && matrix[current_node][i] != -1) {
      if (matrix[current_node][i] < min_weight)
        min_weight = matrix[current_node][i];
    }
  }

  // Graf niespojny lub slepy zaulek
  if (min_weight == numeric_limits<int>::max())
    return;

  for (int i = 0; i < n; ++i) {
    if (!visited[i] && i != current_node &&
        matrix[current_node][i] == min_weight) {
      visited[i] = true;
      exploreRNN(matrix, start_node, i, visited, current_cost + min_weight,
                 visited_count + 1, best_overall_cost, start_time, timeout_ms,
                 time_exceeded);
      visited[i] = false; // Backtracking
    }
  }
}

int repetitiveNearestNeighbour(const vector<vector<int>> &matrix) {
  // najpierw wyliczamy wynik zwyklym NN, dla mocnego upper bound
  int best_cost = nearestNeighbour(matrix);
  int n = matrix.size();

  double timeout_ms = 5000.0; // 5 sekund calosciowego limitu na instancje
  auto start_time = chrono::high_resolution_clock::now();
  bool time_exceeded = false;

  for (int start_node = 0; start_node < n; ++start_node) {
    if (time_exceeded)
      break;
    vector<bool> visited(n, false);
    visited[start_node] = true;
    exploreRNN(matrix, start_node, start_node, visited, 0, 1, best_cost,
               start_time, timeout_ms, time_exceeded);
  }

  return best_cost;
}