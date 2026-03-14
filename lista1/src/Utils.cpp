#include "../include/Utils.hpp"
#include <random>

using namespace std;

vector<vector<int>> generateRandomMatrix(int size, bool symmetric) {
    vector<vector<int>> matrix(size, vector<int>(size, 0));
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 100);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (i == j) {
                matrix[i][j] = -1; // -1 as infinity/no path
            } else {
                if (symmetric) {
                    if (j > i) {
                        int weight = dist(gen);
                        matrix[i][j] = weight;
                        matrix[j][i] = weight;
                    }
                } else {
                    matrix[i][j] = dist(gen);
                }
            }
        }
    }
    return matrix;
}
