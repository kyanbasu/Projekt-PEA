#include "../include/Config.hpp"
#include "../include/TSP.hpp"
#include "../include/Utils.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
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

  if (cfg.test_type & 1) {
    if (cfg.show_progress)
      cout << "Znaleziono " << cfg.instances.size() << " plikow.\n";

    // bufor na wyniki
    ostringstream buf;
    buf << "Instance,Size,Algorithm,Iteration,Time_ms,Cost\n";

    for (const auto &inst_name : cfg.instances) {
      int size = 0;
      string filepath = cfg.data_folder + inst_name;
      vector<vector<int>> matrix = loadMatrix(filepath, size);

      if (cfg.show_progress) {
        cout << "\n============================================\n";
        cout << "Instancja: " << inst_name << " (Rozmiar: " << size << ")\n";
      }

      auto run_rand = [&]() {
        return randomSearch(matrix, cfg.rand_local_repeats);
      };
      auto run_nn = [&]() { return nearestNeighbour(matrix); };
      auto run_rnn = [&]() { return repetitiveNearestNeighbour(matrix); };

      struct AlgoRun {
        string name;
        vector<pair<double, int>> results;
      };

      vector<AlgoRun> algos = {
          {"RAND",
           measureTimeAll(run_rand, cfg.repeats, cfg.show_progress, "RAND")},
          {"NN", measureTimeAll(run_nn, cfg.repeats, cfg.show_progress, "NN")},
          {"RNN",
           measureTimeAll(run_rnn, cfg.repeats, cfg.show_progress, "RNN")},
      };

      long mem_usage = getPeakMemoryUsageKB();

      for (auto &ar : algos) {
        auto [best, avg_t] = summarise(ar.results);

        // zapisanie kazdej iteracji do bufora
        for (int i = 0; i < (int)ar.results.size(); ++i) {
          buf << inst_name << "," << size << "," << ar.name << "," << (i + 1)
              << "," << ar.results[i].first << "," << ar.results[i].second
              << "\n";
        }

        if (cfg.show_progress)
          cout << "   [" << ar.name << "] Avg: " << avg_t
               << " ms | Best: " << best << " | RAM: " << mem_usage << " KB\n";
      }
    }

    // zapisanie bufora do pliku
    ofstream csvOut(cfg.output_file);
    csvOut << buf.str();
    csvOut.close();
    if (cfg.show_progress)
      cout << "\nWyniki zapisano do " << cfg.output_file << "\n";

  } else if (cfg.test_type & 2) {

    if (cfg.show_progress) {
      cout << "\n============================================\n";
      cout << "Brute Force dla grafow losowych o rozmiarach 6 do 15...\n";
    }

    ostringstream bfBuf;
    bfBuf << "Symmetric,Size,BestCost,Time_ms\n";

    for (int size = 6; size <= 15; ++size) {
      vector<vector<int>> matrix_sym = generateRandomMatrix(size, true);
      auto sym_runs = measureTimeAll([&]() { return bruteForce(matrix_sym); },
                                     1, false, "");
      int best_sym = sym_runs[0].second;
      double t_sym = sym_runs[0].first;
      bfBuf << "1," << size << "," << best_sym << "," << t_sym << "\n";
      if (cfg.show_progress)
        cout << "BF Symetryczny (N=" << setw(2) << size << ") -> " << setw(10)
             << t_sym << " ms | Koszt: " << best_sym << "\n";

      vector<vector<int>> matrix_asym = generateRandomMatrix(size, false);
      auto asym_runs = measureTimeAll([&]() { return bruteForce(matrix_asym); },
                                      1, false, "");
      int best_asym = asym_runs[0].second;
      double t_asym = asym_runs[0].first;
      bfBuf << "0," << size << "," << best_asym << "," << t_asym << "\n";
      if (cfg.show_progress)
        cout << "BF Asymetryczny(N=" << setw(2) << size << ") -> " << setw(10)
             << t_asym << " ms | Koszt: " << best_asym << "\n";
    }

    ofstream bfOut("bf_results.csv");
    bfOut << bfBuf.str();
    bfOut.close();

    if (cfg.show_progress)
      cout << "\nZakonczono badanie BruteForce. Wyniki zapisano do "
              "bf_results.csv\n";
  }

  return 0;
}