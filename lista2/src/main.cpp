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
    int cost = -1;
    try {
        cost = function();
    } catch(const std::bad_alloc& e) {
        cost = -2; // Out of memory
    } catch(const std::runtime_error& e) {
        if (std::string(e.what()) == "MemoryLimit") cost = -2;
        else if (std::string(e.what()) == "Timeout") cost = -3;
    }
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
    if (c != -1 && c != -2 && c != -3 && (best == -1 || best == -2 || best == -3 || c < best))
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

    ofstream csvOut(cfg.output_file);
    csvOut << "Instance,Size,Algorithm,Iteration,Time_ms,Cost,MemKB\n";

    for (const auto &inst_name : cfg.instances) {
      int size = 0;
      string filepath = cfg.data_folder + inst_name;
      vector<vector<int>> matrix = loadMatrix(filepath, size);

      if (cfg.show_progress) {
        cout << "\n============================================\n";
        cout << "Instancja: " << inst_name << " (Rozmiar: " << size << ")\n";
      }

      // Upper bound na potrzeby algorytmu B&B
      int ub = numeric_limits<int>::max();
      string ub_name = "INF";
      if(cfg.upper_bound_method == 1) {
          ub = randomSearch(matrix, cfg.rand_local_repeats);
          ub_name = "RAND";
      } else if (cfg.upper_bound_method == 2) {
          ub = nearestNeighbour(matrix);
          ub_name = "NN";
      } else if (cfg.upper_bound_method == 3) {
          ub = repetitiveNearestNeighbour(matrix);
          ub_name = "RNN";
      }
      
      if(cfg.show_progress) cout << "Poczatkowe gorne ograniczenie (" << ub_name << "): " << (ub == numeric_limits<int>::max() ? -1 : ub) << "\n";

      auto run_dfs = [&]() { return branchAndBoundDFS(matrix, ub, cfg.time_limit_min); };
      auto run_lc = [&]() { return branchAndBoundLC(matrix, ub, cfg.time_limit_min); };
      auto run_bfs = [&]() { return branchAndBoundBFS(matrix, ub, cfg.time_limit_min); };

      struct AlgoRun {
        string name;
        vector<pair<double, int>> results;
      };

      vector<AlgoRun> algos = {
          {"BnB_DFS", measureTimeAll(run_dfs, cfg.repeats, cfg.show_progress, "BnB_DFS")},
          {"BnB_LC", measureTimeAll(run_lc, cfg.repeats, cfg.show_progress, "BnB_LC")},
          {"BnB_BFS", measureTimeAll(run_bfs, cfg.repeats, cfg.show_progress, "BnB_BFS")},
      };

      for (auto &ar : algos) {
        auto [best, avg_t] = summarise(ar.results);
        long mem_usage = getPeakMemoryUsageKB(); // szczytowe uzycie rosnie wiec mierzymy po kazdym alg

        // zapisanie kazdej instancji bezposrednio do pliku
        for (int i = 0; i < (int)ar.results.size(); ++i) {
          csvOut << inst_name << "," << size << "," << ar.name << "," << (i + 1)
                 << "," << ar.results[i].first << "," << ar.results[i].second
                 << "," << mem_usage << "\n";
        }
        csvOut.flush();

        if (cfg.show_progress) {
          if (best == -2) {
             cout << "   [" << ar.name << "] Przerwano algorytm z powodu braku pamieci (RAM: " << mem_usage << " KB)\n";
          } else if (best == -3) {
             cout << "   [" << ar.name << "] Przerwano algorytm z powodu przekroczenia limitu " << cfg.time_limit_min << " min\n";
          } else {
             cout << "   [" << ar.name << "] Avg: " << avg_t
                  << " ms | Best: " << best << " | RAM: " << mem_usage << " KB\n";
          }
        }
      }
    }

    csvOut.close();
    if (cfg.show_progress)
      cout << "\nWyniki zapisano do " << cfg.output_file << "\n";

  }

  return 0;
}
