#include "forge_core/submodule/submodule.h"

#include "forge_core/repo/repo.h"
#include "forge_platform/fs.h"

#include <sstream>

namespace forge_core::submodule {

static std::filesystem::path submodules_file(const std::filesystem::path& workdir) {
  return forge_core::repo::make_paths(workdir).forge_dir / "submodules";
}

static std::string trim(std::string s) {
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
  return s;
}

static std::vector<std::string> split(const std::string& s, char delim) {
  std::vector<std::string> result;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    if (!item.empty()) {
      result.push_back(item);
    }
  }
  return result;
}

static std::string join(const std::vector<std::string>& v, char delim) {
  std::string result;
  for (size_t i = 0; i < v.size(); ++i) {
    if (i > 0) result.push_back(delim);
    result.append(v[i]);
  }
  return result;
}

std::vector<Entry> list(const std::filesystem::path& workdir, std::string* err) {
  std::vector<Entry> out;
  auto raw = forge_platform::fs::read_file(submodules_file(workdir));
  if (!raw) return out;

  std::string_view s(*raw);
  size_t pos = 0;
  while (pos < s.size()) {
    auto eol = s.find('\n', pos);
    if (eol == std::string_view::npos) eol = s.size();
    auto line = s.substr(pos, eol - pos);
    auto tab = line.find('\t');
    if (tab != std::string_view::npos) {
      Entry e;
      e.path = std::string(line.substr(0, tab));
      auto rest = line.substr(tab + 1);
      
      auto pipe = rest.find('|');
      if (pipe != std::string_view::npos) {
        e.url = std::string(rest.substr(0, pipe));
        auto sparse_str = std::string(rest.substr(pipe + 1));
        e.sparse_paths = split(sparse_str, ':');
      } else {
        e.url = std::string(rest);
      }
      
      if (!e.path.empty() && !e.url.empty()) out.push_back(std::move(e));
    }
    pos = eol + 1;
  }
  return out;
}

bool add(const std::filesystem::path& workdir, const std::string& url, const std::string& path, std::string* err) {
  return add_sparse(workdir, url, path, {}, err);
}

bool add_sparse(const std::filesystem::path& workdir, const std::string& url, const std::string& path, 
                const std::vector<std::string>& sparse_paths, std::string* err) {
  auto cur = list(workdir, err);
  for (const auto& e : cur) {
    if (e.path == path) {
      if (err) *err = "submodule already exists";
      return false;
    }
  }

  std::string out;
  for (const auto& e : cur) {
    out.append(e.path);
    out.push_back('\t');
    out.append(e.url);
    if (!e.sparse_paths.empty()) {
      out.push_back('|');
      out.append(join(e.sparse_paths, ':'));
    }
    out.push_back('\n');
  }
  out.append(path);
  out.push_back('\t');
  out.append(url);
  if (!sparse_paths.empty()) {
    out.push_back('|');
    out.append(join(sparse_paths, ':'));
  }
  out.push_back('\n');

  if (!forge_platform::fs::write_file_atomic(submodules_file(workdir), out)) {
    if (err) *err = "failed to write submodules";
    return false;
  }
  return true;
}

bool set_sparse_paths(const std::filesystem::path& workdir, const std::string& path, 
                      const std::vector<std::string>& sparse_paths, std::string* err) {
  auto cur = list(workdir, err);
  bool found = false;
  
  for (auto& e : cur) {
    if (e.path == path) {
      e.sparse_paths = sparse_paths;
      found = true;
      break;
    }
  }
  
  if (!found) {
    if (err) *err = "submodule not found";
    return false;
  }

  std::string out;
  for (const auto& e : cur) {
    out.append(e.path);
    out.push_back('\t');
    out.append(e.url);
    if (!e.sparse_paths.empty()) {
      out.push_back('|');
      out.append(join(e.sparse_paths, ':'));
    }
    out.push_back('\n');
  }

  if (!forge_platform::fs::write_file_atomic(submodules_file(workdir), out)) {
    if (err) *err = "failed to write submodules";
    return false;
  }
  return true;
}

bool update(const std::filesystem::path& workdir, const std::string& path, std::string* err) {
  auto entries = list(workdir, err);
  
  for (const auto& e : entries) {
    if (e.path == path) {

      return true;
    }
  }
  
  if (err) *err = "submodule not found";
  return false;
}

}

