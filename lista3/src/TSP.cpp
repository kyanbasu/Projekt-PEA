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

int calculateMST(const vector<vector<int>> &matrix) {
    int n = matrix.size();
    if (n <= 1) return 0;
    
    vector<int> min_e(n, numeric_limits<int>::max());
    vector<bool> in_mst(n, false);
    min_e[0] = 0;
    int total_weight = 0;
    
    for (int i = 0; i < n; ++i) {
        int v = -1;
        for (int j = 0; j < n; ++j) {
            if (!in_mst[j] && (v == -1 || min_e[j] < min_e[v])) {
                v = j;
            }
        }
        if (v == -1 || min_e[v] == numeric_limits<int>::max()) break; // graph not connected
        
        in_mst[v] = true;
        total_weight += min_e[v];
        
        for (int u = 0; u < n; ++u) {
            int weight = matrix[v][u];
            int reverse_weight = matrix[u][v];
            int edge_weight = numeric_limits<int>::max();
            if (weight != -1) edge_weight = min(edge_weight, weight);
            if (reverse_weight != -1) edge_weight = min(edge_weight, reverse_weight);
            
            if (edge_weight != numeric_limits<int>::max() && !in_mst[u] && edge_weight < min_e[u]) {
                min_e[u] = edge_weight;
            }
        }
    }
    return total_weight;
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

double calculateInitialTemperature(const vector<vector<int>> &matrix, const vector<int>& initial_path, int neighbourhood, double target_acceptance = 0.99) {
    int n = matrix.size();
    int samples = 1000;
    if (samples > n * n) samples = n * n;
    
    double sum_positive_delta = 0.0;
    int count_positive = 0;
    
    vector<int> current = initial_path;
    int cost = calculateCost(current, matrix);
    
    for (int i = 0; i < samples; ++i) {
        vector<int> next_path;
        if (neighbourhood == 1) next_path = swapNeighbour(current);
        else if (neighbourhood == 2) next_path = invertNeighbour(current);
        else next_path = insertNeighbour(current);
        
        int next_cost = calculateCost(next_path, matrix);
        if (next_cost != numeric_limits<int>::max()) {
            int delta = next_cost - cost;
            if (delta > 0) {
                sum_positive_delta += delta;
                count_positive++;
            }
        }
        
        // Randomly move to next state to sample space
        if (next_cost != numeric_limits<int>::max()) {
            current = next_path;
            cost = next_cost;
        }
    }
    
    if (count_positive == 0) throw std::runtime_error("Count positive nie może być zerem");
    double avg_delta = sum_positive_delta / count_positive;
    return -avg_delta / log(target_acceptance);
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

    // Zabezpieczenie jesli sciezka ma inny rozmiar
    if ((int)current_path.size() != n) {
        throw std::runtime_error("Sciezka ma rozmiar niezgodny z liczba miast");
    }

    int current_cost = calculateCost(current_path, matrix);

    vector<int> best_path = current_path;
    int best_cost = current_cost;
    
    // Lower Bound z MST
    result.lb = calculateMST(matrix);

    double temperature = initial_temp;
    // Jesli temperatura ujemna, wyznacz automatycznie
    if (temperature < 0) {
        temperature = calculateInitialTemperature(matrix, current_path, neighbourhood, 0.99);
        // Nadpisujemy zmienna local_init_temp na wypadek gdyby schematy chlodzenia z niej korzystaly
        initial_temp = temperature;
    }

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
        if (best_cost <= result.lb) break; // Optymalne rozwiazanie, wczesne wyjscie

        // Schladzanie
        ++iter_count;
        if (cooling_schedule == 0) {
            temperature = coolGeometric(temperature, cooling_rate, iter_count, initial_temp);
        } else if (cooling_schedule == 1) {
            temperature = coolLinear(temperature, cooling_rate, iter_count, initial_temp);
        } else if (cooling_schedule == 2) {
            temperature = coolLogarithmic(temperature, cooling_rate, iter_count, initial_temp);
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
