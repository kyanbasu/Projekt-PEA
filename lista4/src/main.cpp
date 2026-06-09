#include "../include/Config.hpp"
#include "../include/TSP.hpp"
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

string initMethodName(int m) {
    if (m == 1) return "NN";
    if (m == 2) return "RNN";
    return "RAND";
}

int main(int argc, char* argv[]) {
  std::string config_path = "config.ini";
  if (argc > 1) config_path = argv[1];
  Config cfg = loadConfig(config_path);

  // Build combinations for One At a Time (OAT) testing
  struct ParamCombo {
    double alpha;
    double beta;
    double evaporation_rate;
    int ants_count;
    int iterations;
    int init_method;
  };
  vector<ParamCombo> combos;
  
  auto addCombo = [&](double a, double b, double er, int ac, int it, int im) {
      for (const auto& c : combos) {
          if (c.alpha == a && c.beta == b && c.evaporation_rate == er && 
              c.ants_count == ac && c.iterations == it && c.init_method == im) return;
      }
      combos.push_back({a, b, er, ac, it, im});
  };

  // Baseline config (first elements of each list)
  double b_alpha = cfg.aco.alphas.empty() ? 1.0 : cfg.aco.alphas[0];
  double b_beta = cfg.aco.betas.empty() ? 2.0 : cfg.aco.betas[0];
  double b_er = cfg.aco.evaporation_rates.empty() ? 0.1 : cfg.aco.evaporation_rates[0];
  int b_ac = cfg.aco.ants_counts.empty() ? -1 : cfg.aco.ants_counts[0];
  int b_it = cfg.aco.iterations.empty() ? 100 : cfg.aco.iterations[0];
  int b_im = cfg.aco.init_methods.empty() ? 1 : cfg.aco.init_methods[0];

  addCombo(b_alpha, b_beta, b_er, b_ac, b_it, b_im);
  
  for (double a : cfg.aco.alphas) addCombo(a, b_beta, b_er, b_ac, b_it, b_im);
  for (double b : cfg.aco.betas) addCombo(b_alpha, b, b_er, b_ac, b_it, b_im);
  for (double er : cfg.aco.evaporation_rates) addCombo(b_alpha, b_beta, er, b_ac, b_it, b_im);
  for (int ac : cfg.aco.ants_counts) addCombo(b_alpha, b_beta, b_er, ac, b_it, b_im);
  for (int it : cfg.aco.iterations) addCombo(b_alpha, b_beta, b_er, b_ac, it, b_im);
  for (int im : cfg.aco.init_methods) addCombo(b_alpha, b_beta, b_er, b_ac, b_it, im);

  if (cfg.show_progress) {
    cout << "Znaleziono " << cfg.instances.size() << " plikow.\n";
    cout << "Liczba kombinacji parametrow: " << combos.size() << "\n";
    cout << "Laczna liczba uruchomien: " << (cfg.instances.size() * combos.size() * cfg.repeats) << "\n";
  }

  ofstream csvOut(cfg.output_file);
  csvOut << "Instance,Size,Algorithm,InitMethod,"
         << "Alpha,Beta,EvaporationRate,AntsCount,Iterations,"
         << "Repeats,Time_ms,Cost,LB\n";

  for (const auto &inst_name : cfg.instances) {
    int size = 0;
    string filepath = cfg.data_folder + inst_name;
    vector<vector<int>> matrix = loadMatrix(filepath, size);

    if (cfg.show_progress) {
      cout << "\n============================================\n";
      cout << "Instancja: " << inst_name << " (Rozmiar: " << size << ")\n";
    }

    for (const auto &combo : combos) {
      string algo_label = "MMAS_" + initMethodName(combo.init_method);

      int current_lb = 0;
      auto run_aco = [&]() {
          auto res = antColonyOptimization(matrix, combo.alpha, combo.beta, combo.evaporation_rate,
                                           combo.ants_count, combo.iterations, combo.init_method,
                                           cfg.time_limit_min);
          current_lb = res.lb;
          return res.best_cost;
      };

      vector<RunResult> results = measureTimeAll(run_aco, cfg.repeats,
                                                  cfg.show_progress, algo_label);
      auto sum_res = summarise(results);

      for (int i = 0; i < (int)results.size(); ++i) {
                  csvOut << inst_name << ","
                         << size << ","
                         << "MMAS" << ","
                         << initMethodName(combo.init_method) << ","
                         << combo.alpha << ","
                         << combo.beta << ","
                         << combo.evaporation_rate << ","
                         << combo.ants_count << ","
                         << combo.iterations << ","
                         << (i + 1) << ","
                         << results[i].time_ms << ","
                         << results[i].cost << ","
                         << current_lb << "\n";
      }
      csvOut.flush();

      if (cfg.show_progress) {
          if (sum_res.cost == -2) {
             cout << "   [MMAS] Przerwano z powodu braku pamieci\n";
          } else if (sum_res.cost == -3) {
             cout << "   [MMAS] Przerwano z powodu przekroczenia limitu " << cfg.time_limit_min << " min\n";
          } else {
             cout << "   [MMAS] Avg time: " << sum_res.time_ms
                  << " ms | Best cost: " << sum_res.cost
                  << " | LB: " << current_lb
                  << " | Init: " << initMethodName(combo.init_method)
                  << " | Alpha: " << combo.alpha
                  << " | Beta: " << combo.beta
                  << " | Rho: " << combo.evaporation_rate
                  << " | Ants: " << combo.ants_count
                  << " | Iters: " << combo.iterations << "\n";
          }
      }
    }
  }

  csvOut.close();
  if (cfg.show_progress)
    cout << "\nWyniki zapisano do " << cfg.output_file << "\n";

  return 0;
}