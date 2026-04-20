#include "include/Utils.hpp"
#include <fstream>
#include <iostream>

using namespace std;

void saveMatrixTSPLIB(const vector<vector<int>>& mat, const string& name, bool is_sym) {
    ofstream out("data_generated/" + name);
    int n = mat.size();
    out << "NAME: " << name << "\n";
    out << "TYPE: " << (is_sym ? "TSP" : "ATSP") << "\n";
    out << "DIMENSION: " << n << "\n";
    out << "EDGE_WEIGHT_TYPE: EXPLICIT\n";
    out << "EDGE_WEIGHT_FORMAT: FULL_MATRIX\n";
    out << "EDGE_WEIGHT_SECTION\n";
    for(int i=0; i<n; ++i) {
        for(int j=0; j<n; ++j) {
            out << (mat[i][j] == -1 ? 999999 : mat[i][j]) << " ";
        }
        out << "\n";
    }
    out << "EOF\n";
    out.close();
}

int main() {
    vector<int> sizes = {6, 8, 10, 12, 13, 14, 15};
    for(int s : sizes) {
        auto sym_mat = generateRandomMatrix(s, true);
        saveMatrixTSPLIB(sym_mat, "sym_0" + (s < 10 ? "0" + to_string(s) : to_string(s)) + ".tsp", true);
        auto asym_mat = generateRandomMatrix(s, false);
        saveMatrixTSPLIB(asym_mat, "asym_0" + (s < 10 ? "0" + to_string(s) : to_string(s)) + ".atsp", false);
    }
    cout << "Wygenerowano.\n";
    return 0;
}
