#include "../include/Config.hpp"
#include "../include/TSP.hpp"
#include "../include/Utils.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

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
};

template <typename Func>
vector<RunResult> measureTimeAll(Func function, int repeats,
                                         bool show_progress,
                                         const string &algo_name) {
  vector<RunResult> results;
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

RunResult summarise(const vector<RunResult> &runs) {
  int best = runs[0].cost;
  double sum_t = 0.0;
  for (const auto &run : runs) {
    if (run.cost != -1 && run.cost != -2 && run.cost != -3 && (best == -1 || best == -2 || best == -3 || run.cost < best))
      best = run.cost;
    sum_t += run.time_ms;
  }
  return {sum_t / runs.size(), best};
}

string neighbourhoodName(int n) {
    if (n == 1) return "SWAP";
    if (n == 2) return "INVERT";
    return "INSERT";
}

string initMethodName(int m) {
    if (m == 1) return "NN";
    if (m == 2) return "RNN";
    return "RAND";
}

int main(int argc, char* argv[]) {
  std::string config_path = "config.ini";
  if (argc > 1) config_path = argv[1];
  Config cfg = loadConfig(config_path);

  // Build all parameter combinations
  struct ParamCombo {
    double cooling_rate;
    double initial_temp;
    double final_temp;
    int iter_per_temp;
    int neighbourhood;
    int init_method;
    int cooling_schedule;
  };
  vector<ParamCombo> combos;
  for (double cr : cfg.sa.cooling_rates)
    for (double it : cfg.sa.initial_temps)
      for (double ft : cfg.sa.final_temps)
        for (int ipt : cfg.sa.iter_per_temps)
          for (int neigh : cfg.sa.neighbourhoods)
            for (int init : cfg.sa.init_methods)
              for (int cs : cfg.sa.cooling_schedules)
                combos.push_back({cr, it, ft, ipt, neigh, init, cs});

  if (cfg.show_progress) {
    cout << "Znaleziono " << cfg.instances.size() << " plikow.\n";
    cout << "Liczba kombinacji parametrow: " << combos.size() << "\n";
    cout << "Laczna liczba uruchomien: " << (cfg.instances.size() * combos.size() * cfg.repeats) << "\n";
  }

  ofstream csvOut(cfg.output_file);
  csvOut << "Instance,Size,Algorithm,InitMethod,Neighbourhood,"
         << "CoolingRate,InitTemp,FinalTemp,IterPerTemp,CoolingSchedule,"
         << "Repeats,Time_ms,Cost\n";

  for (const auto &inst_name : cfg.instances) {
    int size = 0;
    string filepath = cfg.data_folder + inst_name;
    vector<vector<int>> matrix = loadMatrix(filepath, size);

    if (cfg.show_progress) {
      cout << "\n============================================\n";
      cout << "Instancja: " << inst_name << " (Rozmiar: " << size << ")\n";
    }

    for (const auto &combo : combos) {
      string algo_label = "SA_" + initMethodName(combo.init_method)
                        + "_" + neighbourhoodName(combo.neighbourhood);

      auto run_sa = [&]() {
          auto res = simulatedAnnealing(matrix, combo.initial_temp,
                                        combo.final_temp, combo.cooling_rate,
                                        combo.iter_per_temp, combo.neighbourhood,
                                        combo.init_method, cfg.time_limit_min,
                                        combo.cooling_schedule);
          return res.best_cost;
      };

      vector<RunResult> results = measureTimeAll(run_sa, cfg.repeats,
                                                  cfg.show_progress, algo_label);
      auto sum_res = summarise(results);

      for (int i = 0; i < (int)results.size(); ++i) {
                  csvOut << inst_name << ","
                         << size << ","
                         << "SA" << ","
                         << initMethodName(combo.init_method) << ","
                         << neighbourhoodName(combo.neighbourhood) << ","
                         << combo.cooling_rate << ","
                         << combo.initial_temp << ","
                         << combo.final_temp << ","
                         << combo.iter_per_temp << ","
                         << combo.cooling_schedule << ","
                         << (i + 1) << ","
                         << results[i].time_ms << ","
                         << results[i].cost << "\n";
      }
      csvOut.flush();

      if (cfg.show_progress) {
          if (sum_res.cost == -2) {
             cout << "   [SA] Przerwano z powodu braku pamieci\n";
          } else if (sum_res.cost == -3) {
             cout << "   [SA] Przerwano z powodu przekroczenia limitu " << cfg.time_limit_min << " min\n";
          } else {
             cout << "   [SA] Avg time: " << sum_res.time_ms
                  << " ms | Best cost: " << sum_res.cost
                  << " | Init: " << initMethodName(combo.init_method)
                  << " | Neighbour: " << neighbourhoodName(combo.neighbourhood)
                  << " | CS: " << combo.cooling_schedule
                  << " | IPT: " << combo.iter_per_temp
                  << " | TF: " << combo.final_temp
                  << " | T0: " << combo.initial_temp
                  << " | CR: " << combo.cooling_rate << "\n";
          }
      }
    }
  }

  csvOut.close();
  if (cfg.show_progress)
    cout << "\nWyniki zapisano do " << cfg.output_file << "\n";

  return 0;
}