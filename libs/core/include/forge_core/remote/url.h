#pragma once

#include <optional>
#include <string>

namespace forge_core::remote {

enum class Scheme {
  File,
  Http,
  Https,
  Ssh
};

struct RemoteUrl {
  Scheme scheme = Scheme::File;
  std::string host;
  int port = 0;
  std::string user;
  std::string path;
  std::string raw;
};

std::optional<RemoteUrl> parse_url(const std::string& s);
std::string normalize_path(const std::string& s);

}

