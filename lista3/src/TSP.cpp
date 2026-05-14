#include "../include/TSP.hpp"
#include <algorithm>
#include <limits>
#include <numeric>
#include <random>
#include <chrono>
#include <stdexcept>
#include <cmath>

using namespace std;

// Globalny generator liczb losowych (deterministyczne ziarno dla powtarzalnosci)
static mt19937 rng(42);

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

void fisherYatesShuffle(vector<int>::iterator first, vector<int>::iterator last, mt19937 &gen) {
  int n = distance(first, last);
  if (n <= 1) return;
  for (int i = n - 1; i > 0; --i) {
    uniform_int_distribution<int> dist(0, i);
    int j = dist(gen);
    iter_swap(first + i, first + j);
  }
}

int randomSearch(const vector<vector<int>> &matrix, int local_repeats) {
  int size = matrix.size();
  vector<int> path(size);
  iota(path.begin(), path.end(), 0);

  int min_cost = numeric_limits<int>::max();

  for (int i = 0; i < local_repeats; ++i) {
    fisherYatesShuffle(path.begin() + 1, path.end(), rng);
    int current_cost = calculateCost(path, matrix);
    if (current_cost < min_cost) min_cost = current_cost;
  }
  return min_cost;
}

int nearestNeighbour(const vector<vector<int>> &matrix) {
  int n = matrix.size();
  vector<bool> visited(n, false);
  int current_node = 0; 
  visited[current_node] = true;

  int total_cost = 0;

  for (int step = 1; step < n; ++step) {
    int next_node = -1;
    int min_weight = numeric_limits<int>::max();

    for (int i = 0; i < n; ++i) {
      if (!visited[i] && matrix[current_node][i] != -1 && matrix[current_node][i] < min_weight) {
        min_weight = matrix[current_node][i];
        next_node = i;
      }
    }
    if (next_node == -1) return numeric_limits<int>::max();

    visited[next_node] = true;
    total_cost += min_weight;
    current_node = next_node;
  }

  if (matrix[current_node][0] == -1) return numeric_limits<int>::max();
  total_cost += matrix[current_node][0];
  return total_cost;
}

int repetitiveNearestNeighbour(const vector<vector<int>> &matrix) {
  int best_cost = numeric_limits<int>::max();
  int n = matrix.size();

  for (int start_node = 0; start_node < n; ++start_node) {
    vector<bool> visited(n, false);
    int current_node = start_node;
    visited[current_node] = true;
    int total_cost = 0;   
    bool valid = true;

    for (int step = 1; step < n; ++step) {
        int next_node = -1;
        int min_weight = numeric_limits<int>::max();
    
        for (int i = 0; i < n; ++i) {
          if (!visited[i] && matrix[current_node][i] != -1 && matrix[current_node][i] < min_weight) {
            min_weight = matrix[current_node][i];
            next_node = i;
          }
        }
        if (next_node == -1) { valid = false; break; }
        
        visited[next_node] = true;
        total_cost += min_weight;
        current_node = next_node;
    }
    if (valid) {
        if (matrix[current_node][start_node] == -1) valid = false;
        else total_cost += matrix[current_node][start_node];
    }

    if (valid && total_cost < best_cost) best_cost = total_cost;
  }

  return best_cost;
}

// --- Generowanie rozwiazania poczatkowego ---

vector<int> generateRandomPath(int size) {
    vector<int> path(size);
    iota(path.begin(), path.end(), 0);
    fisherYatesShuffle(path.begin() + 1, path.end(), rng);
    return path;
}

vector<int> nearestNeighbourPath(const vector<vector<int>> &matrix) {
  int n = matrix.size();
  vector<bool> visited(n, false);
  vector<int> path;
  int current_node = 0;
  visited[current_node] = true;
  path.push_back(current_node);

  for (int step = 1; step < n; ++step) {
    int next_node = -1;
    int min_weight = numeric_limits<int>::max();

    for (int i = 0; i < n; ++i) {
      if (!visited[i] && matrix[current_node][i] != -1 && matrix[current_node][i] < min_weight) {
        min_weight = matrix[current_node][i];
        next_node = i;
      }
    }
    if (next_node == -1) break; // cannot continue

    visited[next_node] = true;
    path.push_back(next_node);
    current_node = next_node;
  }
  return path;
}

vector<int> repetitiveNearestNeighbourPath(const vector<vector<int>> &matrix) {
  int n = matrix.size();
  vector<int> best_path;
  int best_cost = numeric_limits<int>::max();

  for (int start_node = 0; start_node < n; ++start_node) {
    vector<bool> visited(n, false);
    vector<int> path;
    int current_node = start_node;
    visited[current_node] = true;
    path.push_back(current_node);
    int total_cost = 0;
    bool valid = true;

    for (int step = 1; step < n; ++step) {
        int next_node = -1;
        int min_weight = numeric_limits<int>::max();
        for (int i = 0; i < n; ++i) {
          if (!visited[i] && matrix[current_node][i] != -1 && matrix[current_node][i] < min_weight) {
            min_weight = matrix[current_node][i];
            next_node = i;
          }
        }
        if (next_node == -1) { valid = false; break; }
        visited[next_node] = true;
        path.push_back(next_node);
        total_cost += min_weight;
        current_node = next_node;
    }
    if (valid) {
        if (matrix[current_node][start_node] == -1) valid = false;
        else total_cost += matrix[current_node][start_node];
    }

    if (valid && total_cost < best_cost) {
        best_cost = total_cost;
        best_path = path;
    }
  }

  if (best_path.empty()) {
      // fallback: generate random
      best_path = generateRandomPath(n);
  }
  return best_path;
}

// --- Operacje sasiedztwa ---

vector<int> swapNeighbour(const vector<int>& path) {
    vector<int> neighbour = path;
    int n = path.size();
    if (n < 3) return neighbour;

    uniform_int_distribution<int> dist(1, n - 1);
    int i = dist(rng);
    int j = dist(rng);
    while (i == j) j = dist(rng);

    swap(neighbour[i], neighbour[j]);
    return neighbour;
}

vector<int> invertNeighbour(const vector<int>& path) {
    vector<int> neighbour = path;
    int n = path.size();
    if (n < 3) return neighbour;

    uniform_int_distribution<int> dist(1, n - 1);
    int i = dist(rng);
    int j = dist(rng);
    while (i == j) j = dist(rng);

    if (i > j) swap(i, j);
    reverse(neighbour.begin() + i, neighbour.begin() + j + 1);
    return neighbour;
}

vector<int> insertNeighbour(const vector<int>& path) {
    vector<int> neighbour = path;
    int n = path.size();
    if (n < 3) return neighbour;

    uniform_int_distribution<int> dist(1, n - 1);
    int i = dist(rng);
    int j = dist(rng);
    while (i == j) j = dist(rng);

    int city = neighbour[i];
    neighbour.erase(neighbour.begin() + i);
    neighbour.insert(neighbour.begin() + j, city);
    return neighbour;
}

// --- Symulowane wyzarzanie (SA) ---
// Schematy chlodzenia: 0=geometryczny, 1=liniowy, 2=logarytmiczny
static double coolGeometric(double temp, double rate, int /*iter*/, double /*init_temp*/) {
    return temp * rate;
}
static double coolLinear(double temp, double rate, int /*iter*/, double init_temp) {
    return temp - (init_temp * (1.0 - rate));
}
static double coolLogarithmic(double temp, double rate, int iter, double init_temp) {
    if (iter <= 0) return init_temp;
    return init_temp / (1.0 + rate * log(1.0 + iter));
}

SAResult simulatedAnnealing(const std::vector<std::vector<int>>& matrix,
                            double initial_temp, double final_temp,
                            double cooling_rate, int iter_per_temp,
                            int neighbourhood, int init_method,
                            int time_limit_min, int cooling_schedule) {
    SAResult result;
    int n = matrix.size();

    // Generowanie rozwiazania poczatkowego
    vector<int> current_path;
    if (init_method == 1) {
        current_path = nearestNeighbourPath(matrix);
    } else if (init_method == 2) {
        current_path = repetitiveNearestNeighbourPath(matrix);
    } else {
        current_path = generateRandomPath(n);
    }

    // Zabezpieczenie: jesli sciezka ma inny rozmiar, wygeneruj losowa
    if ((int)current_path.size() != n) {
        current_path = generateRandomPath(n);
    }

    int current_cost = calculateCost(current_path, matrix);

    vector<int> best_path = current_path;
    int best_cost = current_cost;

    double temperature = initial_temp;

    double timeout_ms = time_limit_min * 60.0 * 1000.0;
    auto start_time = chrono::high_resolution_clock::now();
    int64_t iter_count = 0;
    int64_t inner_iter = 0;
    bool timeout_hit = false;
    int64_t max_epochs = 500000; // zabezpieczenie przed nieskonczona petla (logarytmiczny)

    // Zapisujemy historie kosztu dla analizy
    result.cost_history.push_back(current_cost);

    while (temperature > final_temp && iter_count < max_epochs) {
        for (int i = 0; i < iter_per_temp; ++i) {
            ++inner_iter;
            if ((inner_iter & 127) == 0) {
                auto now = chrono::high_resolution_clock::now();
                chrono::duration<double, milli> elapsed = now - start_time;
                if (elapsed.count() > timeout_ms) {
                    timeout_hit = true;
                    break;
                }
            }

            // Generowanie sasiada
            vector<int> new_path;
            if (neighbourhood == 1) {
                new_path = swapNeighbour(current_path);
            } else if (neighbourhood == 2) {
                new_path = invertNeighbour(current_path);
            } else {
                new_path = insertNeighbour(current_path);
            }

            int new_cost = calculateCost(new_path, matrix);
            if (new_cost == numeric_limits<int>::max()) continue; // niepoprawna sciezka

            int delta = new_cost - current_cost;

            if (delta < 0) {
                // Akceptuj lepsze rozwiazanie
                current_path = new_path;
                current_cost = new_cost;

                if (current_cost < best_cost) {
                    best_path = current_path;
                    best_cost = current_cost;
                }
            } else {
                // Akceptuj gorsze z pewnym prawdopodobienstwem
                double acceptance = exp(-delta / temperature);
                uniform_real_distribution<double> dist(0.0, 1.0);
                if (dist(rng) < acceptance) {
                    current_path = new_path;
                    current_cost = new_cost;
                }
            }
        }

        if (timeout_hit) break;

        // Schladzanie
        ++iter_count;
        if (cooling_schedule == 0) {
            temperature = coolGeometric(temperature, cooling_rate, iter_count, initial_temp);
        } else if (cooling_schedule == 1) {
            temperature = coolLinear(temperature, cooling_rate, iter_count, initial_temp);
        } else if (cooling_schedule == 2) {
            temperature = coolLogarithmic(temperature, cooling_rate, 0, initial_temp);
        }
        
        if (temperature < final_temp) temperature = final_temp;

        // Co pewien czas zapisz historie
        if ((iter_count & 3) == 0) {
            result.cost_history.push_back(best_cost);
        }
    }

    if (timeout_hit) {
        throw std::runtime_error("Timeout");
    }

    result.best_path = best_path;
    result.best_cost = best_cost;
    result.mem_kb = 0; // Pomijalne, poniewaz SA nie uzywa zadnych istotnie duzych struktur

    return result;
}
