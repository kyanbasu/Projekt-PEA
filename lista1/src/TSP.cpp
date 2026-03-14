#include "../include/TSP.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <random>
#include <limits>
#include <cmath>

using namespace std;

int calculateEuc2D(const Node& a, const Node& b) {
    double xd = a.x - b.x;
    double yd = a.y - b.y;
    return static_cast<int>(round(sqrt(xd * xd + yd * yd)));
}

vector<vector<int>> loadMatrix(const string& filepath, int& size) {
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
        // Ignorujemy puste linie lub EOF marker
        if (line.empty() || line.find("EOF") != string::npos) continue;

        // Szukamy wymiaru
        if (line.find("DIMENSION") != string::npos) {
            auto colon_pos = line.find(':');
            if (colon_pos != string::npos) {
                size = stoi(line.substr(colon_pos + 1));
            } else {
                // Jesli nie ma dwukropka np. 'DIMENSION 17'
                stringstream ss(line);
                string dummy;
                ss >> dummy >> size;
            }
            continue;
        }

        // Sprawdzamy typ wag
        if (line.find("EDGE_WEIGHT_TYPE") != string::npos) {
            if (line.find("EUC_2D") != string::npos) {
                is_euc2d = true;
            }
            continue;
        }

        // Szukamy startu sekcji danych
        if (line.find("NODE_COORD_SECTION") != string::npos) {
            in_coord_section = true;
            in_matrix_section = false;
            continue;
        } else if (line.find("EDGE_WEIGHT_SECTION") != string::npos) {
            in_matrix_section = true;
            in_coord_section = false;
            continue;
        }

        // Parsowanie wlasciwych danych
        if (in_coord_section) {
            stringstream ss(line);
            Node n;
            if (ss >> n.id >> n.x >> n.y) {
                nodes.push_back(n);
            }
        } else if (in_matrix_section) {
            // Znalazlismy poczatek macierzy. Wychodzimy z tej petli by wczytac calosc strumieniem.
            break; 
        }
    }

    vector<vector<int>> matrix(size, vector<int>(size, 0));

    // Jeśli to były punkty (EUC_2D), generujemy z nich macierz
    if (is_euc2d && !nodes.empty()) {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (i == j) {
                    matrix[i][j] = 0; // Lub wysoka wartosc, jesli TSPLIB uzywa 9999
                } else {
                    matrix[i][j] = calculateEuc2D(nodes[i], nodes[j]);
                }
            }
        }
    } 
    // Jeśli to była pełna macierz (EDGE_WEIGHT_SECTION)
    else {
        // Wznawiamy wczytywanie od momentu znalezienia sekcji EDGE_WEIGHT
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                file >> matrix[i][j];
            }
        }
    }

    return matrix;
}

int calculateCost(const vector<int> &path, const vector<vector<int>> &matrix)
{
    int cost = 0;
    for (size_t i = 0; i < path.size() - 1; ++i)
    {
        cost += matrix[path[i]][path[i + 1]];
    }
    cost += matrix[path.back()][path[0]];
    return cost;
}

// --- Brute Force ---
int bruteForce(const vector<vector<int>> &matrix)
{
    int size = matrix.size();
    vector<int> path(size);
    iota(path.begin(), path.end(), 0);

    int min_cost = numeric_limits<int>::max();
    do
    {
        int current_cost = calculateCost(path, matrix);
        if (current_cost < min_cost)
            min_cost = current_cost;
    } while (next_permutation(path.begin() + 1, path.end()));
    // +1 by zawsze zaczynac z zera (optymalizacja dla symetrycznych/cykli)

    return min_cost;
}

// --- RAND ---
int randomSearch(const vector<vector<int>> &matrix, int local_repeats)
{
    int size = matrix.size();
    vector<int> path(size);
    iota(path.begin(), path.end(), 0);

    random_device rd;
    mt19937 g(rd());
    int min_cost = numeric_limits<int>::max();

    for (int i = 0; i < local_repeats; ++i)
    {
        shuffle(path.begin() + 1, path.end(), g);
        int current_cost = calculateCost(path, matrix);
        if (current_cost < min_cost)
            min_cost = current_cost;
    }

    return min_cost;
}

// --- Nearest Neighbour (NN) ---
int nearestNeighbour(const vector<vector<int>> &matrix)
{
    int n = matrix.size();
    vector<bool> visited(n, false);
    int current_node = 0; // Zaczynamy sztywno od węzła 0
    visited[current_node] = true;

    int total_cost = 0;

    for (int step = 1; step < n; ++step)
    {
        int next_node = -1;
        int min_weight = numeric_limits<int>::max();

        for (int i = 0; i < n; ++i)
        {
            if (!visited[i] && matrix[current_node][i] < min_weight)
            {
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

// --- Repetitive Nearest Neighbour (RNN) ---
int repetitiveNearestNeighbour(const vector<vector<int>> &matrix)
{
    int best_cost = numeric_limits<int>::max();
    int n = matrix.size();

    for (int start_node = 0; start_node < n; ++start_node)
    {
        vector<bool> visited(n, false);
        int current_node = start_node;
        visited[current_node] = true;

        int total_cost = 0;

        for (int step = 1; step < n; ++step)
        {
            int next_node = -1;
            int min_weight = numeric_limits<int>::max();

            for (int i = 0; i < n; ++i)
            {
                if (!visited[i] && matrix[current_node][i] < min_weight)
                {
                    min_weight = matrix[current_node][i];
                    next_node = i;
                }
            }

            if (next_node != -1) {
                visited[next_node] = true;
                total_cost += min_weight;
                current_node = next_node;
            }
        }

        total_cost += matrix[current_node][start_node]; // Powrot do startu
        if (total_cost < best_cost) {
            best_cost = total_cost;
        }
    }
    return best_cost;
}