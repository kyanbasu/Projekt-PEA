#include "../include/Config.hpp"
#include "../include/TSP.hpp"
#include "../include/Utils.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void printProgress(int current, int total, const string &prefix) {
  int barWidth = 50;
  float progress = static_cast<float>(current) / total;
  int pos = static_cast<int>(barWidth * progress);

  cout << "\r" << setw(15) << left << prefix << " [";
  for (int i = 0; i < barWidth; ++i) {
    if (i < pos)
      cout << "=";
    else if (i == pos)
      cout << ">";
    else
      cout << " ";
  }
  cout << "] " << int(progress * 100.0) << " %" << flush;

  if (current == total)
    cout << endl;
}

// Makro do mierzenia czasu
template <typename Func>
vector<pair<double, int>> measureTimeAll(Func function, int repeats,
                                         bool show_progress,
                                         const string &algo_name) {
  vector<pair<double, int>> results;
  results.reserve(repeats);

  for (int r = 0; r < repeats; ++r) {
    auto t0 = chrono::high_resolution_clock::now();
    int cost = function();
    auto t1 = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> dur = t1 - t0;
    results.push_back({dur.count(), cost});

    if (show_progress)
      printProgress(r + 1, repeats, algo_name);
  }
  return results;
}

// funkcja pomocicza do obliczania najlepszego kosztu i sredniego czasu z
// wektora wynikow
pair<int, double> summarise(const vector<pair<double, int>> &runs) {
  int best = runs[0].second;
  double sum_t = 0.0;
  for (const auto &[t, c] : runs) {
    if (c != -1 && (best == -1 || c < best))
      best = c;
    sum_t += t;
  }
  return {best, sum_t / runs.size()};
}

int main() {
  Config cfg = loadConfig("config.ini");


  return 0;
}