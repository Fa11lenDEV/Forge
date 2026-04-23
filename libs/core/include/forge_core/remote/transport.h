#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace forge_core::remote {

struct RemoteSpec {
  std::string url;
  std::string token;
};

struct FetchResult {
  bool ok = false;
  std::string err;
};

struct PushResult {
  bool ok = false;
  std::string err;
};

FetchResult fetch_into_repo(const std::filesystem::path& workdir, const RemoteSpec& remote);
PushResult push_from_repo(const std::filesystem::path& workdir, const RemoteSpec& remote);

}

