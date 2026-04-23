#include "forge_cli/args.h"

#include <cstring>

namespace forge_cli {

static bool starts_with(const char* s, const char* prefix) {
  return std::strncmp(s, prefix, std::strlen(prefix)) == 0;
}

bool ParsedArgs::has_flag(std::string_view name) const {
  auto it = options.find(std::string(name));
  return it != options.end() && it->second == "1";
}

std::string ParsedArgs::option(std::string_view name, std::string_view def) const {
  auto it = options.find(std::string(name));
  if (it == options.end()) return std::string(def);
  return it->second;
}

ParsedArgs parse_args(int argc, char** argv) {
  ParsedArgs out;
  if (argc <= 1) return out;

  int i = 1;
  if (i < argc && argv[i][0] != '-') {
    out.command = argv[i++];
  }

  for (; i < argc; ++i) {
    const char* a = argv[i];
    if (!starts_with(a, "--")) {
      out.positionals.emplace_back(a);
      continue;
    }

    const char* eq = std::strchr(a, '=');
    if (!eq) {
      std::string key = std::string(a + 2);
      out.options.emplace(std::move(key), "1");
      continue;
    }

    std::string key(a + 2, eq);
    std::string val(eq + 1);
    out.options.emplace(std::move(key), std::move(val));
  }

  return out;
}

}

