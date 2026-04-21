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

struct RunResult {
    double time_ms;
    int cost;
    long mem_kb;
};

// Makro do mierzenia czasu
template <typename Func>
vector<RunResult> measureTimeAll(Func function, int repeats,
                                         bool show_progress,
                                         const string &algo_name) {
  vector<RunResult> results;
  results.reserve(repeats);

  for (int r = 0; r < repeats; ++r) {
    auto t0 = chrono::high_resolution_clock::now();
    int cost = -1;
    long mem_kb = 0;
    try {
        auto res = function();
        cost = res.first;
        mem_kb = res.second;
    } catch(const std::bad_alloc& e) {
        cost = -2; // Out of memory
    } catch(const std::runtime_error& e) {
        if (std::string(e.what()) == "MemoryLimit") cost = -2;
        else if (std::string(e.what()) == "Timeout") cost = -3;
    }
    auto t1 = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> dur = t1 - t0;
    results.push_back({dur.count(), cost, mem_kb});

    if (show_progress)
      printProgress(r + 1, repeats, algo_name);
  }
  return results;
}

// funkcja pomocicza do obliczania najlepszego kosztu i sredniego czasu z
// wektora wynikow
RunResult summarise(const vector<RunResult> &runs) {
  int best = runs[0].cost;
  double sum_t = 0.0;
  long max_mem = 0;
  for (const auto &run : runs) {
    if (run.cost != -1 && run.cost != -2 && run.cost != -3 && (best == -1 || best == -2 || best == -3 || run.cost < best))
      best = run.cost;
    sum_t += run.time_ms;
    if (run.mem_kb > max_mem) max_mem = run.mem_kb;
  }
  return {sum_t / runs.size(), best, max_mem};
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

      auto run_dfs = [&]() { return branchAndBoundDFS(matrix, ub, cfg.time_limit_min, cfg.memory_limit_mb); };
      auto run_lc = [&]() { return branchAndBoundLC(matrix, ub, cfg.time_limit_min, cfg.memory_limit_mb); };
      auto run_bfs = [&]() { return branchAndBoundBFS(matrix, ub, cfg.time_limit_min, cfg.memory_limit_mb); };

      struct AlgoRun {
        string name;
        vector<RunResult> results;
      };

      vector<AlgoRun> algos = {
          {"BnB_DFS", measureTimeAll(run_dfs, cfg.repeats, cfg.show_progress, "BnB_DFS")},
          {"BnB_LC", measureTimeAll(run_lc, cfg.repeats, cfg.show_progress, "BnB_LC")},
          {"BnB_BFS", measureTimeAll(run_bfs, cfg.repeats, cfg.show_progress, "BnB_BFS")},
      };

      for (auto &ar : algos) {
        auto sum_res = summarise(ar.results);

        // zapisanie kazdej instancji bezposrednio do pliku
        for (int i = 0; i < (int)ar.results.size(); ++i) {
          csvOut << inst_name << "," << size << "," << ar.name << "," << (i + 1)
                 << "," << ar.results[i].time_ms << "," << ar.results[i].cost
                 << "," << ar.results[i].mem_kb << "\n";
        }
        csvOut.flush();

        if (cfg.show_progress) {
          if (sum_res.cost == -2) {
             cout << "   [" << ar.name << "] Przerwano algorytm z powodu braku pamieci (> " << cfg.memory_limit_mb << " MB)\n";
          } else if (sum_res.cost == -3) {
             cout << "   [" << ar.name << "] Przerwano algorytm z powodu przekroczenia limitu " << cfg.time_limit_min << " min\n";
          } else {
             cout << "   [" << ar.name << "] Avg: " << sum_res.time_ms
                  << " ms | Best: " << sum_res.cost << " | RAM: " << sum_res.mem_kb << " KB\n";
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
