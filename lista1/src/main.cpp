#include "../include/Config.hpp"
#include "../include/TSP.hpp"
#include "../include/Utils.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

// Funkcja rysujaca pasek postepu w konsoli
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
double measureTime(Func function, int repeats, int &best_cost_out,
                   bool show_progress, const string &algo_name) {
  auto start = chrono::high_resolution_clock::now();

  for (int r = 0; r < repeats; ++r) {
    best_cost_out = function();

    if (show_progress) {
      printProgress(r + 1, repeats, algo_name);
    }
  }

  auto end = chrono::high_resolution_clock::now();
  chrono::duration<double, milli> duration = end - start;
  return duration.count() / repeats;
}

int main() {
  Config cfg = loadConfig("config.ini");

  if (cfg.test_type & 1) {

    ofstream csvOut(cfg.output_file);
    csvOut << "Instance,Size,Algorithm,BestCost,AvgTime_ms,PeakMem_KB\n";
    csvOut.flush();

    if (cfg.show_progress)
      cout << "Znaleziono " << cfg.instances.size() << " plikow.\n";

    for (const auto &inst_name : cfg.instances) {
      int size = 0;
      string filepath = cfg.data_folder + inst_name;
      vector<vector<int>> matrix = loadMatrix(filepath, size);

      if (cfg.show_progress) {
        cout << "\n============================================\n";
        cout << "Instancja: " << inst_name << " (Rozmiar: " << size << ")\n";
      }

      int best_cost;
      double avg_time;
      long mem_usage;

      // 1. Brute Force
      if (size <= 12) {
        best_cost = -1;
        // Dla BF dajemy 1 powtorzenie w measureTime, bo to przeglad zupelny i
        // trwa dlugo. Pasek postepu skoczy od razu do 100% po zakonczeniu.
        avg_time = measureTime([&]() { return bruteForce(matrix); }, 1,
                               best_cost, cfg.show_progress, "BruteForce");
        mem_usage = getPeakMemoryUsageKB();

        csvOut << inst_name << "," << size << ",BruteForce," << best_cost << ","
               << avg_time << "," << mem_usage << "\n";
        csvOut.flush(); // Natychmiastowy zapis do pliku!

        if (cfg.show_progress)
          cout << "   -> Czas: " << avg_time << " ms | Koszt: " << best_cost
               << " | RAM: " << mem_usage << " KB\n";
      } else {
        if (cfg.show_progress)
          cout << "BruteForce      [ POMINIETO (rozmiar > 12) ]\n";
      }

      // 2. RAND
      best_cost = -1;
      avg_time = measureTime(
          [&]() { return randomSearch(matrix, cfg.rand_local_repeats); },
          cfg.repeats, best_cost, cfg.show_progress, "RAND");
      mem_usage = getPeakMemoryUsageKB();

      csvOut << inst_name << "," << size << ",RAND," << best_cost << ","
             << avg_time << "," << mem_usage << "\n";
      csvOut.flush();
      if (cfg.show_progress)
        cout << "   -> Czas: " << avg_time << " ms | Koszt: " << best_cost
             << " | RAM: " << mem_usage << " KB\n";

      // 3. Nearest Neighbour (NN)
      best_cost = -1;
      avg_time = measureTime([&]() { return nearestNeighbour(matrix); },
                             cfg.repeats, best_cost, cfg.show_progress, "NN");
      mem_usage = getPeakMemoryUsageKB();

      csvOut << inst_name << "," << size << ",NN," << best_cost << ","
             << avg_time << "," << mem_usage << "\n";
      csvOut.flush();
      if (cfg.show_progress)
        cout << "   -> Czas: " << avg_time << " ms | Koszt: " << best_cost
             << " | RAM: " << mem_usage << " KB\n";

      // 4. Repetitive Nearest Neighbour (RNN)
      best_cost = -1;
      avg_time =
          measureTime([&]() { return repetitiveNearestNeighbour(matrix); },
                      cfg.repeats, best_cost, cfg.show_progress, "RNN");
      mem_usage = getPeakMemoryUsageKB();

      csvOut << inst_name << "," << size << ",RNN," << best_cost << ","
             << avg_time << "," << mem_usage << "\n";
      csvOut.flush();
      if (cfg.show_progress)
        cout << "   -> Czas: " << avg_time << " ms | Koszt: " << best_cost
             << " | RAM: " << mem_usage << " KB\n";
    }

    csvOut.close();
  } else if (cfg.test_type & 2) {

    // Testowanie Brute Force na losowych grafach (Rozmiar 6-15)
    if (cfg.show_progress) {
      cout << "\n============================================\n";
      cout << "Rozpoczynam badanie Brute Force dla grafow losowych o "
              "rozmiarach 6 do 15...\n";
    }

    ofstream bfOut("bf_results.csv");
    bfOut << "Symmetric,Size,BestCost,Time_ms\n";

    for (int size = 6; size <= 15; ++size) {
      // Grafy symetryczne
      vector<vector<int>> matrix_sym = generateRandomMatrix(size, true);
      int best_cost = -1;
      double avg_time = measureTime([&]() { return bruteForce(matrix_sym); }, 1,
                                    best_cost, false, "");
      bfOut << "1," << size << "," << best_cost << "," << avg_time << "\n";
      if (cfg.show_progress)
        cout << "BF Symetryczny (Rozmiar " << setw(2) << size
             << ") -> Czas: " << setw(10) << avg_time
             << " ms | Koszt: " << best_cost << "\n";

      // Grafy asymetryczne
      vector<vector<int>> matrix_asym = generateRandomMatrix(size, false);
      best_cost = -1;
      avg_time = measureTime([&]() { return bruteForce(matrix_asym); }, 1,
                             best_cost, false, "");
      bfOut << "0," << size << "," << best_cost << "," << avg_time << "\n";
      if (cfg.show_progress)
        cout << "BF Asymetryczny(Rozmiar " << setw(2) << size
             << ") -> Czas: " << setw(10) << avg_time
             << " ms | Koszt: " << best_cost << "\n";
    }

    bfOut.close();

    if (cfg.show_progress)
      cout << "\nZakonczono badanie BruteForce pomyslnie. Wyniki sa w "
              "bf_results.csv\n";
    if (cfg.show_progress)
      cout << "Glowne wyliczenia wykonane prawidlowo.\n";
  }

  return 0;
}