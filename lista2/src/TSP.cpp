#include "../include/TSP.hpp"
#include <algorithm>
#include <limits>
#include <numeric>
#include <random>
#include <queue>
#include <stack>
#include <chrono>
#include <stdexcept>

using namespace std;

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

void fisherYatesShuffle(vector<int>::iterator first, vector<int>::iterator last, mt19937 &rng) {
  int n = distance(first, last);
  if (n <= 1) return;
  for (int i = n - 1; i > 0; --i) {
    uniform_int_distribution<int> dist(0, i);
    int j = dist(rng);
    iter_swap(first + i, first + j);
  }
}

int randomSearch(const vector<vector<int>> &matrix, int local_repeats) {
  int size = matrix.size();
  vector<int> path(size);
  iota(path.begin(), path.end(), 0);

  static mt19937 g(42);
  int min_cost = numeric_limits<int>::max();

  for (int i = 0; i < local_repeats; ++i) {
    fisherYatesShuffle(path.begin() + 1, path.end(), g);
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

struct BnBNode {
    vector<vector<int>> matrix; 
    vector<int> path; 
    int cost; 
    int current_city;
    int level; 
};

struct CompareBnBNode {
    bool operator()(const BnBNode& a, const BnBNode& b) const {
        return a.cost > b.cost;
    }
};

// Redukcja macierzy Little'a i obliczanie dodatku kosztowego do LB
int reduceMatrix(vector<vector<int>>& mat) {
    int reduction_sum = 0;
    int n = mat.size();

    for (int i = 0; i < n; ++i) {
        int min_val = numeric_limits<int>::max();
        for (int j = 0; j < n; ++j) {
            if (mat[i][j] != -1 && mat[i][j] < min_val) min_val = mat[i][j];
        }
        if (min_val != numeric_limits<int>::max() && min_val > 0) {
            reduction_sum += min_val;
            for (int j = 0; j < n; ++j) {
                if (mat[i][j] != -1) mat[i][j] -= min_val;
            }
        }
    }

    for (int j = 0; j < n; ++j) {
        int min_val = numeric_limits<int>::max();
        for (int i = 0; i < n; ++i) {
            if (mat[i][j] != -1 && mat[i][j] < min_val) min_val = mat[i][j];
        }
        if (min_val != numeric_limits<int>::max() && min_val > 0) {
            reduction_sum += min_val;
            for (int i = 0; i < n; ++i) {
                if (mat[i][j] != -1) mat[i][j] -= min_val;
            }
        }
    }
    return reduction_sum;
}

BnBNode createRootNode(const vector<vector<int>>& original_mat) {
    BnBNode root;
    root.matrix = original_mat;
    root.path.push_back(0); // Start z wierzcholka 0
    root.current_city = 0;
    root.level = 1;
    for(int i = 0; i < (int)root.matrix.size(); ++i) root.matrix[i][i] = -1; // Blokada powrotu do siebie samego (nie potrzebne jesli to 0)
    root.cost = reduceMatrix(root.matrix);
    return root;
}

BnBNode createChildNode(const BnBNode& parent, int next_city) {
    BnBNode child;
    child.matrix = parent.matrix;
    child.path = parent.path;
    child.path.push_back(next_city);
    child.current_city = next_city;
    child.level = parent.level + 1;

    int edge_cost = parent.matrix[parent.current_city][next_city];
    
    // Zablokuj wiersz skad wyruszamy oraz kolumne do ktorej dotarlismy
    int n = child.matrix.size();
    for (int k = 0; k < n; ++k) {
        child.matrix[parent.current_city][k] = -1;
        child.matrix[k][next_city] = -1;
    }
    // Blokada powrotu aby nie stworzyc wewnetrznego cyklu
    child.matrix[next_city][child.path[0]] = -1;

    int reduction = reduceMatrix(child.matrix);
    child.cost = parent.cost + edge_cost + reduction;

    return child;
}

// --- BRANCH AND BOUND: Wersja BFS (Breadth-First Search) ---
int branchAndBoundBFS(const vector<vector<int>>& matrix, int initial_upper_bound, int time_limit_min) {
    int best_cost = initial_upper_bound;
    queue<BnBNode> q;

    double timeout_ms = time_limit_min * 60.0 * 1000.0;
    auto start_time = chrono::high_resolution_clock::now();
    int iter_count = 0;
    const size_t MAX_QUEUE_SIZE = 2000000;

    BnBNode root = createRootNode(matrix);
    if(root.cost >= best_cost) return best_cost;
    q.push(root);

    int n = matrix.size();

    while (!q.empty()) {
        if ((++iter_count & 1023) == 0) {
            auto now = chrono::high_resolution_clock::now();
            chrono::duration<double, milli> elapsed = now - start_time;
            if (elapsed.count() > timeout_ms) throw std::runtime_error("Timeout");
        }
        if (q.size() > MAX_QUEUE_SIZE) throw std::runtime_error("MemoryLimit");

        BnBNode current = q.front();
        q.pop();

        if (current.cost >= best_cost) continue;

        if (current.level == n) {
            int final_edge = current.matrix[current.current_city][current.path[0]];
            int final_cost = current.cost;
            if(final_edge != -1) final_cost += final_edge;
            
            if (final_cost < best_cost) best_cost = final_cost;
            continue;
        }

        for (int i = 0; i < n; ++i) {
            if (current.matrix[current.current_city][i] != -1) {
                BnBNode child = createChildNode(current, i);
                if (child.cost < best_cost) {
                    q.push(child);
                }
            }
        }
    }

    return best_cost;
}

int branchAndBoundDFS(const vector<vector<int>>& matrix, int initial_upper_bound, int time_limit_min) {
    int best_cost = initial_upper_bound;
    stack<BnBNode> s;

    double timeout_ms = time_limit_min * 60.0 * 1000.0;
    auto start_time = chrono::high_resolution_clock::now();
    int iter_count = 0;
    const size_t MAX_QUEUE_SIZE = 2000000;

    BnBNode root = createRootNode(matrix);
    if(root.cost >= best_cost) return best_cost;
    s.push(root);

    int n = matrix.size();

    while (!s.empty()) {
        if ((++iter_count & 1023) == 0) {
            auto now = chrono::high_resolution_clock::now();
            chrono::duration<double, milli> elapsed = now - start_time;
            if (elapsed.count() > timeout_ms) throw std::runtime_error("Timeout");
        }
        if (s.size() > MAX_QUEUE_SIZE) throw std::runtime_error("MemoryLimit");

        BnBNode current = s.top();
        s.pop();

        if (current.cost >= best_cost) continue;

        if (current.level == n) {
            int final_edge = current.matrix[current.current_city][current.path[0]];
            int final_cost = current.cost;
            if(final_edge != -1) final_cost += final_edge;
            
            if (final_cost < best_cost) best_cost = final_cost;
            continue;
        }

        // Dodawanie w odwrotnej kolejnosci nie ma wplywu na poprawnosc, ale mozna dodawac normalnie
        for (int i = 0; i < n; ++i) {
            if (current.matrix[current.current_city][i] != -1) {
                BnBNode child = createChildNode(current, i);
                if (child.cost < best_cost) {
                    s.push(child);
                }
            }
        }
    }

    return best_cost;
}

// --- BRANCH AND BOUND: Wersja Lowest-Cost (Best-First Search) ---
int branchAndBoundLC(const vector<vector<int>>& matrix, int initial_upper_bound, int time_limit_min) {
    int best_cost = initial_upper_bound;
    priority_queue<BnBNode, vector<BnBNode>, CompareBnBNode> pq;

    double timeout_ms = time_limit_min * 60.0 * 1000.0;
    auto start_time = chrono::high_resolution_clock::now();
    int iter_count = 0;
    const size_t MAX_QUEUE_SIZE = 2000000;

    BnBNode root = createRootNode(matrix);
    if(root.cost >= best_cost) return best_cost;
    pq.push(root);

    int n = matrix.size();

    while (!pq.empty()) {
        if ((++iter_count & 1023) == 0) {
            auto now = chrono::high_resolution_clock::now();
            chrono::duration<double, milli> elapsed = now - start_time;
            if (elapsed.count() > timeout_ms) throw std::runtime_error("Timeout");
        }
        if (pq.size() > MAX_QUEUE_SIZE) throw std::runtime_error("MemoryLimit");

        BnBNode current = pq.top();
        pq.pop();

        if (current.cost >= best_cost) continue;

        if (current.level == n) {
            int final_edge = current.matrix[current.current_city][current.path[0]];
            int final_cost = current.cost;
            if(final_edge != -1) final_cost += final_edge;
            
            if (final_cost < best_cost) best_cost = final_cost;
            continue;
        }

        for (int i = 0; i < n; ++i) {
            if (current.matrix[current.current_city][i] != -1) {
                BnBNode child = createChildNode(current, i);
                if (child.cost < best_cost) {
                    pq.push(child);
                }
            }
        }
    }

    return best_cost;
}
