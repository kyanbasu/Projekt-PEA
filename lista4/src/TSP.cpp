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
        if (v == -1 || min_e[v] == numeric_limits<int>::max()) break;
        
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
    if (next_node == -1) break;

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

// --- Algorytm Mrowkowy (ACO i MMAS) ---

ACOResult antColonyOptimization(const std::vector<std::vector<int>>& matrix,
                                double alpha, double beta, double evaporation_rate,
                                int ants_count, int iterations,
                                int init_method, int aco_variant, int time_limit_min) {
    ACOResult result;
    int n = matrix.size();
    if (n <= 1) {
        throw std::invalid_argument("Macierz musi miec co najmniej 2 miasta.");
    }
    
    if (ants_count == -1) ants_count = n; // Dynamic ants_count

    if (ants_count <= 0 || iterations <= 0 || alpha < 0 || beta < 0 || evaporation_rate <= 0 || evaporation_rate > 1) {
        throw std::invalid_argument("Nieprawidlowe parametry ACO.");
    }

    result.lb = calculateMST(matrix);

    // Heurystyka: eta = 1 / d
    std::vector<std::vector<double>> eta(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && matrix[i][j] != -1 && matrix[i][j] != 0) {
                eta[i][j] = 1.0 / matrix[i][j];
            } else if (i != j && matrix[i][j] == 0) {
                eta[i][j] = 1e9; // ekstremalnie duza heurystyka dla zerowej odleglosci
            }
        }
    }

    // Inicjalizacja feromonu
    std::vector<int> initial_path;
    if (init_method == 1) {
        initial_path = nearestNeighbourPath(matrix);
    } else if (init_method == 2) {
        initial_path = repetitiveNearestNeighbourPath(matrix);
    } else {
        initial_path = generateRandomPath(n);
    }
    
    if ((int)initial_path.size() != n) {
        throw std::runtime_error("Sciezka inicjalizacyjna ma niepoprawny rozmiar.");
    }

    int initial_cost = calculateCost(initial_path, matrix);
    if (initial_cost == std::numeric_limits<int>::max()) {
         throw std::runtime_error("Poczatkowa trasa nie istnieje.");
    }

    std::vector<int> global_best_path = initial_path;
    int global_best_cost = initial_cost;

    // Inicjalizacja feromonu w zaleznosci od wariantu
    double tau_max = 1.0;
    double tau_min = 0.0;
    double initial_tau = 1.0;
    
    if (aco_variant == 1) { // MMAS
        tau_max = 1.0 / (evaporation_rate * global_best_cost);
        tau_min = tau_max / (2.0 * n);
        initial_tau = tau_max;
    } else { // Basic AS
        initial_tau = (double)ants_count / global_best_cost;
    }
    
    std::vector<std::vector<double>> tau(n, std::vector<double>(n, initial_tau));
    std::vector<std::vector<double>> choice_info(n, std::vector<double>(n, 0.0));

    double timeout_ms = time_limit_min * 60.0 * 1000.0;
    // Precompute Candidate Lists (nn_list)
    int nn_size = std::min(n - 1, 30); // lista najblizszych sasiadow (max 30)
    std::vector<std::vector<int>> nn_list(n, std::vector<int>(nn_size));
    for (int i = 0; i < n; ++i) {
        std::vector<std::pair<int, int>> dists;
        dists.reserve(n);
        for (int j = 0; j < n; ++j) {
            if (i != j && matrix[i][j] != -1) {
                dists.push_back({matrix[i][j], j});
            }
        }
        std::sort(dists.begin(), dists.end());
        for (int k = 0; k < nn_size && k < (int)dists.size(); ++k) {
            nn_list[i][k] = dists[k].second;
        }
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    bool timeout_hit = false;

    std::uniform_real_distribution<double> dist_01(0.0, 1.0);
    std::uniform_int_distribution<int> dist_node(0, n - 1);

    for (int iter = 0; iter < iterations; ++iter) {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - start_time;
        if (elapsed.count() > timeout_ms) {
            timeout_hit = true;
            break;
        }

        // Precomputing choice_info for the entire iteration
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j && matrix[i][j] != -1) {
                    choice_info[i][j] = std::pow(tau[i][j], alpha) * std::pow(eta[i][j], beta);
                } else {
                    choice_info[i][j] = 0.0;
                }
            }
        }

        std::vector<std::vector<int>> ant_paths(ants_count, std::vector<int>(n));
        std::vector<int> ant_costs(ants_count, 0);

        for (int k = 0; k < ants_count; ++k) {
            std::vector<bool> visited(n, false);
            
            int start_node = dist_node(rng);
            ant_paths[k][0] = start_node;
            visited[start_node] = true;

            int current_node = start_node;
            bool path_valid = true;

            for (int step = 1; step < n; ++step) {
                int next_node = -1;
                double sum_prob = 0.0;
                
                // Szybki wybor z listy kandydatow (Candidate List)
                std::vector<int> available_nn;
                available_nn.reserve(nn_size);
                for (int nn_node : nn_list[current_node]) {
                    if (!visited[nn_node]) {
                        available_nn.push_back(nn_node);
                    }
                }

                if (!available_nn.empty()) {
                    std::vector<double> probs(available_nn.size());
                    for (size_t i = 0; i < available_nn.size(); ++i) {
                        probs[i] = choice_info[current_node][available_nn[i]];
                        sum_prob += probs[i];
                    }

                    if (sum_prob > 0) {
                        double r = dist_01(rng) * sum_prob;
                        double cumulative = 0.0;
                        for (size_t i = 0; i < available_nn.size(); ++i) {
                            cumulative += probs[i];
                            if (cumulative >= r) {
                                next_node = available_nn[i];
                                break;
                            }
                        }
                    }
                }

                // Jesli lista kandydatow zawiodla (wszyscy odwiedzeni), robimy klasyczny O(n) fallback
                if (next_node == -1) {
                    sum_prob = 0.0;
                    std::vector<int> remaining;
                    remaining.reserve(n);
                    for (int j = 0; j < n; ++j) {
                        if (!visited[j] && matrix[current_node][j] != -1) {
                            remaining.push_back(j);
                        }
                    }
                    
                    if (!remaining.empty()) {
                        std::vector<double> probs(remaining.size());
                        for (size_t i = 0; i < remaining.size(); ++i) {
                            probs[i] = choice_info[current_node][remaining[i]];
                            sum_prob += probs[i];
                        }
                        if (sum_prob > 0) {
                            double r = dist_01(rng) * sum_prob;
                            double cumulative = 0.0;
                            for (size_t i = 0; i < remaining.size(); ++i) {
                                cumulative += probs[i];
                                if (cumulative >= r) {
                                    next_node = remaining[i];
                                    break;
                                }
                            }
                        }
                        if (next_node == -1) {
                            next_node = remaining.back();
                        }
                    }
                }

                if (next_node == -1) {
                    path_valid = false;
                    break;
                }

                ant_paths[k][step] = next_node;
                visited[next_node] = true;
                ant_costs[k] += matrix[current_node][next_node];
                
                current_node = next_node;
            }

            if (path_valid) {
                if (matrix[current_node][start_node] == -1) {
                    path_valid = false;
                } else {
                    ant_costs[k] += matrix[current_node][start_node];
                }
            } else {
                ant_costs[k] = std::numeric_limits<int>::max();
            }

            if (path_valid && ant_costs[k] < global_best_cost) {
                global_best_cost = ant_costs[k];
                global_best_path = ant_paths[k];
                
                if (aco_variant == 1) { // MMAS
                    // MMAS aktualizuje limity po znalezieniu nowego the best
                    tau_max = 1.0 / (evaporation_rate * global_best_cost);
                    tau_min = tau_max / (2.0 * n);
                }
            }
        }

        // Parowanie feromonu ze wszystkich krawedzi (wspolne dla AS i MMAS)
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                tau[i][j] = (1.0 - evaporation_rate) * tau[i][j];
            }
        }
        
        if (aco_variant == 1) { // MMAS
            // Zostawianie feromonu tylko przez Global Best
            if (global_best_cost != std::numeric_limits<int>::max()) {
                double delta_tau = 1.0 / global_best_cost;
                for (int i = 0; i < n - 1; ++i) {
                    int u = global_best_path[i];
                    int v = global_best_path[i + 1];
                    tau[u][v] += delta_tau;
                }
                int u = global_best_path.back();
                int v = global_best_path[0];
                tau[u][v] += delta_tau;
            }

            // Przycinanie feromonu do limitow MMAS
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    if (tau[i][j] > tau_max) tau[i][j] = tau_max;
                    if (tau[i][j] < tau_min) tau[i][j] = tau_min;
                }
            }
        } else { // Basic AS
            // Zostawianie feromonu przez wszystkie mrowki z biezacej iteracji
            for (int k = 0; k < ants_count; ++k) {
                if (ant_costs[k] != std::numeric_limits<int>::max()) {
                    double delta_tau = 1.0 / ant_costs[k];
                    for (int i = 0; i < n - 1; ++i) {
                        int u = ant_paths[k][i];
                        int v = ant_paths[k][i + 1];
                        tau[u][v] += delta_tau;
                    }
                    int u = ant_paths[k].back();
                    int v = ant_paths[k][0];
                    tau[u][v] += delta_tau;
                }
            }
        }

        if ((iter & 7) == 0) {
             result.cost_history.push_back(global_best_cost);
        }
        
        if (global_best_cost <= result.lb) {
            break; // Osiagnieto optimalny koszt, wczesne wyjscie
        }
    }

    if (timeout_hit) {
        throw std::runtime_error("Timeout");
    }

    result.best_path = global_best_path;
    result.best_cost = global_best_cost;
    result.mem_kb = 0; 
    return result;
}
