#include "../include/Config.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

std::vector<double> parseDoubleList(const std::string& val) {
    std::vector<double> result;
    std::stringstream ss(val);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t\r\n"));
        token.erase(token.find_last_not_of(" \t\r\n") + 1);
        if (!token.empty()) {
            result.push_back(std::stod(token));
        }
    }
    return result;
}

std::vector<int> parseIntList(const std::string& val) {
    std::vector<int> result;
    std::stringstream ss(val);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" \t\r\n"));
        token.erase(token.find_last_not_of(" \t\r\n") + 1);
        if (!token.empty()) {
            result.push_back(std::stoi(token));
        }
    }
    return result;
}

Config loadConfig(const std::string &filename) {
  Config cfg;
  cfg.aco.alphas = {1.0};
  cfg.aco.betas = {2.0};
  cfg.aco.evaporation_rates = {0.1};
  cfg.aco.ants_counts = {-1}; // domyslnie -1 oznacza dynamicznie (rowne N)
  cfg.aco.iterations = {1000};
  cfg.aco.init_methods = {1}; // domyslnie NN
  cfg.time_limit_min = 5;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "BLAD: Nie mozna otworzyc pliku konfiguracyjnego " << filename
              << "!\n";
    exit(1);
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    auto delimiterPos = line.find('=');
    if (delimiterPos != std::string::npos) {
      std::string key = line.substr(0, delimiterPos);
      std::string val = line.substr(delimiterPos + 1);

      if (key == "data_folder")
        cfg.data_folder = val;
      else if (key == "output_file")
        cfg.output_file = val;
      else if (key == "show_progress")
        cfg.show_progress = std::stoi(val);
      else if (key == "repeats")
        cfg.repeats = std::stoi(val);
      else if (key == "alpha")
        cfg.aco.alphas = parseDoubleList(val);
      else if (key == "beta")
        cfg.aco.betas = parseDoubleList(val);
      else if (key == "evaporation_rate")
        cfg.aco.evaporation_rates = parseDoubleList(val);
      else if (key == "ants_count")
        cfg.aco.ants_counts = parseIntList(val);
      else if (key == "iterations")
        cfg.aco.iterations = parseIntList(val);
      else if (key == "init_method")
        cfg.aco.init_methods = parseIntList(val);
      else if (key == "time_limit_min")
        cfg.time_limit_min = std::stoi(val);
    }
  }

  // Szukanie plikow .tsp oraz .atsp
  if (fs::exists(cfg.data_folder) && fs::is_directory(cfg.data_folder)) {
    for (const auto &entry : fs::directory_iterator(cfg.data_folder)) {
      std::string ext = entry.path().extension().string();
      if (ext == ".tsp" || ext == ".atsp" || ext == ".txt") {
        cfg.instances.push_back(entry.path().filename().string());
      }
    }
  } else {
    std::cerr << "BLAD: Folder z danymi nie istnieje: " << cfg.data_folder
              << "\n";
    exit(1);
  }

  return cfg;
}

int calculateEuc2D(const Node &a, const Node &b) {
  double xd = a.x - b.x;
  double yd = a.y - b.y;
  return static_cast<int>(std::round(std::sqrt(xd * xd + yd * yd)));
}

int calculateGeoDist(const Node &a, const Node &b) {
  double PI = 3.141592;
  double degToRad = PI / 180.0;

  double lat_a = degToRad * a.x;
  double lon_a = degToRad * a.y;
  double lat_b = degToRad * b.x;
  double lon_b = degToRad * b.y;

  double q1 = cos(lon_a - lon_b);
  double q2 = cos(lat_a - lat_b);
  double q3 = cos(lat_a + lat_b);
  return static_cast<int>(std::round(6378.388 * std::acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0));
}

std::vector<std::vector<int>> loadMatrix(const std::string &filepath,
                                         int &size) {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    std::cerr << "BLAD: Nie znaleziono pliku " << filepath << "\n";
    exit(1);
  }

  std::string line;
  bool is_euc2d = false;
  bool is_geo = false;
  bool in_coord_section = false;
  std::string weight_format = "FULL_MATRIX";
  std::vector<Node> nodes;

  size = 0;

  while (std::getline(file, line)) {
    if (line.empty() || line.find("EOF") != std::string::npos)
      continue;

    // proba zaladowania zwyklej macierzy
    if (size == 0 &&
        line.find_first_not_of(" \t\r\n0123456789") == std::string::npos) {
      std::stringstream ss(line);
      if (ss >> size) {
        break;
      }
    }

    // rozmiar
    if (line.find("DIMENSION") != std::string::npos) {
      auto colon_pos = line.find(':');
      if (colon_pos != std::string::npos) {
        size = std::stoi(line.substr(colon_pos + 1));
      } else {
        std::stringstream ss(line);
        std::string dummy;
        ss >> dummy >> size;
      }
      continue;
    }

    // format zapisu wag
    if (line.find("EDGE_WEIGHT_FORMAT") != std::string::npos) {
      if (line.find("LOWER_DIAG_ROW") != std::string::npos) {
        weight_format = "LOWER_DIAG_ROW";
      } else if (line.find("UPPER_ROW") != std::string::npos) {
        weight_format = "UPPER_ROW";
      } else if (line.find("FULL_MATRIX") != std::string::npos) {
        weight_format = "FULL_MATRIX";
      }
      continue;
    }

    // typ wag
    if (line.find("EDGE_WEIGHT_TYPE") != std::string::npos) {
      if (line.find("EUC_2D") != std::string::npos) {
        is_euc2d = true;
      } else if (line.find("GEO") != std::string::npos) {
        is_geo = true;
      }
      continue;
    }

    // koordynaty
    if (line.find("NODE_COORD_SECTION") != std::string::npos) {
      in_coord_section = true;
      continue;
    } else if (line.find("EDGE_WEIGHT_SECTION") != std::string::npos) {
      break; // Zatrzymujemy parsujaca petle getline na poczatku sekcji macierzy
    }

    if (in_coord_section) {
      std::stringstream ss(line);
      Node n;
      if (ss >> n.id >> n.x >> n.y) {
        nodes.push_back(n);
      }
    }
  }

  std::vector<std::vector<int>> matrix(size, std::vector<int>(size, 0));

  if (is_geo && !nodes.empty()) {
    for (int i = 0; i < size; ++i) {
      for (int j = 0; j < size; ++j) {
        if (i == j) {
          matrix[i][j] = 0;
        } else {
          matrix[i][j] = calculateGeoDist(nodes[i], nodes[j]);
        }
      }
    }
  } else if (is_euc2d && !nodes.empty()) {
    for (int i = 0; i < size; ++i) {
      for (int j = 0; j < size; ++j) {
        if (i == j) {
          matrix[i][j] = 0;
        } else {
          matrix[i][j] = calculateEuc2D(nodes[i], nodes[j]);
        }
      }
    }
  } else {
    if (weight_format == "LOWER_DIAG_ROW") {
      for (int i = 0; i < size; ++i) {
        for (int j = 0; j <= i; ++j) {
          file >> matrix[i][j];
          matrix[j][i] = matrix[i][j]; // odbicie symetryczne
        }
      }
    } else if (weight_format == "UPPER_ROW") {
      for (int i = 0; i < size - 1; ++i) {
        for (int j = i + 1; j < size; ++j) {
          file >> matrix[i][j];
          matrix[j][i] = matrix[i][j]; // odbicie symetryczne
        }
      }
      for (int i = 0; i < size; ++i)
        matrix[i][i] = 0; // diagonal zerowany
    } else {
      // domyślnie FULL_MATRIX
      for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
          file >> matrix[i][j];
        }
      }
    }
  }

  return matrix;
}