#include "forge_cli/args.h"
#include "forge_core/submodule/submodule.h"
#include "forge_platform/path.h"

#include <iostream>
#include <sstream>

namespace forge_app::commands {

static std::vector<std::string> parse_sparse_paths(const std::string& s) {
  std::vector<std::string> result;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, ',')) {
    if (!item.empty()) {
      result.push_back(item);
    }
  }
  return result;
}

int submodule(const forge_cli::ParsedArgs& a) {
  auto sub = a.positionals.empty() ? "status" : a.positionals[0];
  std::string err;
  auto wd = forge_platform::path::cwd();

  if (sub == "add") {
    if (a.positionals.size() < 3) {
      std::cerr << "forge submodule add: use <url> <path> [--sparse=path1,path2,...]\n";
      std::cerr << "  --sparse: Clone only specified paths (comma-separated)\n";
      std::cerr << "  Example: forge submodule add https://repo.com/engine.git engine --sparse=src/core,include\n";
      return 2;
    }
    auto url = a.positionals[1];
    auto path = a.positionals[2];
    
    std::vector<std::string> sparse_paths;
    auto sparse_it = a.options.find("sparse");
    if (sparse_it != a.options.end() && !sparse_it->second.empty()) {
      sparse_paths = parse_sparse_paths(sparse_it->second);
    }
    
    if (!sparse_paths.empty()) {
      if (!forge_core::submodule::add_sparse(wd, url, path, sparse_paths, &err)) {
        std::cerr << "forge submodule: " << err << "\n";
        return 1;
      }
      std::cout << "Submodule added with sparse checkout: ";
      for (size_t i = 0; i < sparse_paths.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << sparse_paths[i];
      }
      std::cout << "\n";
    } else {
      if (!forge_core::submodule::add(wd, url, path, &err)) {
        std::cerr << "forge submodule: " << err << "\n";
        return 1;
      }
    }
    return 0;
  }

  if (sub == "set-sparse") {
    if (a.positionals.size() < 3) {
      std::cerr << "forge submodule set-sparse: use <path> <sparse_paths>\n";
      std::cerr << "  Example: forge submodule set-sparse engine src/core,include\n";
      return 2;
    }
    auto path = a.positionals[1];
    auto sparse_str = a.positionals[2];
    auto sparse_paths = parse_sparse_paths(sparse_str);
    
    if (!forge_core::submodule::set_sparse_paths(wd, path, sparse_paths, &err)) {
      std::cerr << "forge submodule: " << err << "\n";
      return 1;
    }
    std::cout << "Sparse paths updated for " << path << "\n";
    return 0;
  }

  if (sub == "update") {
    if (a.positionals.size() < 2) {
      std::cerr << "forge submodule update: use <path>\n";
      return 2;
    }
    auto path = a.positionals[1];
    if (!forge_core::submodule::update(wd, path, &err)) {
      std::cerr << "forge submodule: " << err << "\n";
      return 1;
    }
    return 0;
  }

  if (sub == "status") {
    auto list = forge_core::submodule::list(wd, &err);
    for (const auto& e : list) {
      std::cout << e.path << " " << e.url;
      if (!e.sparse_paths.empty()) {
        std::cout << " [sparse: ";
        for (size_t i = 0; i < e.sparse_paths.size(); ++i) {
          if (i > 0) std::cout << ", ";
          std::cout << e.sparse_paths[i];
        }
        std::cout << "]";
      }
      std::cout << "\n";
    }
    return 0;
  }

  std::cerr << "forge submodule: unknown subcommand\n";
  std::cerr << "Available commands:\n";
  std::cerr << "  add <url> <path> [--sparse=paths]  - Add a submodule\n";
  std::cerr << "  set-sparse <path> <sparse_paths>   - Set sparse checkout for a submodule\n";
  std::cerr << "  update <path>                      - Update a submodule\n";
  std::cerr << "  status                             - List all submodules\n";
  return 2;
}

}

