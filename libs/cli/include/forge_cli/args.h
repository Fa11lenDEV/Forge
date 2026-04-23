#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace forge_cli {

struct ParsedArgs {
  std::string command;
  std::vector<std::string> positionals;
  std::unordered_map<std::string, std::string> options;
  bool has_flag(std::string_view name) const;
  std::string option(std::string_view name, std::string_view def = {}) const;
};

ParsedArgs parse_args(int argc, char** argv);

}

