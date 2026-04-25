#include "forge_cli/args.h"
#include "forge_core/commit/commit.h"
#include "forge_core/checkout/checkout.h"
#include "forge_core/index/index.h"
#include "forge_core/remote/transport.h"
#include "forge_core/remote/url.h"
#include "forge_core/repo/repo.h"
#include "forge_core/refs/refs.h"
#include "forge_platform/fs.h"

#include <git2.h>

#include <chrono>
#include <filesystem>
#include <iostream>

namespace forge_app::commands {

static bool copy_dir(const std::filesystem::path& from, const std::filesystem::path& to, std::string* err) {
  std::error_code ec;
  std::filesystem::create_directories(to, ec);
  if (ec) { if (err) *err = "failed to create directory"; return false; }

  for (auto& it : std::filesystem::recursive_directory_iterator(from, ec)) {
    if (ec) break;
    auto rel = it.path().lexically_relative(from);
    auto dst = to / rel;
    if (it.is_directory()) {
      std::filesystem::create_directories(dst, ec);
      if (ec) { if (err) *err = "failed to create directory"; return false; }
      continue;
    }
    if (it.is_regular_file()) {
      std::filesystem::create_directories(dst.parent_path(), ec);
      std::filesystem::copy_file(it.path(), dst, std::filesystem::copy_options::overwrite_existing, ec);
      if (ec) { if (err) *err = "failed to copy file"; return false; }
    }
  }
  return true;
}

static bool copy_worktree_without_git(const std::filesystem::path& from, const std::filesystem::path& to, std::string* err) {
  std::error_code ec;
  std::filesystem::create_directories(to, ec);
  if (ec) { if (err) *err = "failed to create destination"; return false; }

  for (auto& it : std::filesystem::recursive_directory_iterator(from, ec)) {
    if (ec) break;
    auto rel = it.path().lexically_relative(from);
    if (rel.empty()) continue;
    if (rel.begin()->string() == ".git") continue;
    auto dst = to / rel;
    if (it.is_directory()) {
      std::filesystem::create_directories(dst, ec);
      if (ec) { if (err) *err = "failed to create directory"; return false; }
      continue;
    }
    if (it.is_regular_file()) {
      std::filesystem::create_directories(dst.parent_path(), ec);
      std::filesystem::copy_file(it.path(), dst, std::filesystem::copy_options::overwrite_existing, ec);
      if (ec) { if (err) *err = "failed to copy file"; return false; }
    }
  }
  return true;
}

static bool infer_git_remote(const std::string& spec) {
  if (spec.rfind("git@", 0) == 0) return true;
  if (spec.rfind("ssh://", 0) == 0) return true;
  if (spec.rfind("git://", 0) == 0) return true;
  if (spec.size() >= 4 && spec.substr(spec.size() - 4) == ".git") return true;
  return false;
}

static bool import_git_worktree_as_forge(const std::filesystem::path& src_worktree,
                                         const std::filesystem::path& dest,
                                         const std::string& remote_spec,
                                         std::string* err) {
  if (!copy_worktree_without_git(src_worktree, dest, err)) return false;

  if (!forge_core::repo::init(dest, err)) return false;

  std::vector<std::string> paths = {"."};
  if (!forge_core::index::add_paths(dest, paths, err)) return false;

  auto c = forge_core::commit::create_commit_from_index(dest, "import from git", err);
  if (c.commit_hex.empty()) return false;

  if (!forge_platform::fs::ensure_dir(dest / ".forge" / "remotes")) {
    if (err) *err = "failed to create remotes dir";
    return false;
  }
  if (!forge_platform::fs::write_file_atomic(dest / ".forge" / "remotes" / "origin", remote_spec + "\n")) {
    if (err) *err = "failed to write origin remote";
    return false;
  }
  return true;
}

struct GitAuthPayload {
  std::string token;
};

struct GitProgressPayload {
  int last_percent = -1;
};

static int git_credential_cb(git_credential** out,
                             const char*,
                             const char*,
                             unsigned int allowed_types,
                             void* payload) {
  auto* auth = static_cast<GitAuthPayload*>(payload);
  if (!auth || auth->token.empty()) return GIT_PASSTHROUGH;
  if (allowed_types & GIT_CREDENTIAL_USERPASS_PLAINTEXT) {
    return git_credential_userpass_plaintext_new(out, "x-access-token", auth->token.c_str());
  }
  return GIT_PASSTHROUGH;
}

static int git_transfer_progress_cb(const git_indexer_progress* stats, void* payload) {
  if (!stats) return 0;
  auto* p = static_cast<GitProgressPayload*>(payload);
  int percent = 0;
  if (stats->total_objects > 0) {
    percent = static_cast<int>((100.0 * static_cast<double>(stats->received_objects)) /
                               static_cast<double>(stats->total_objects));
  }
  if (!p || percent != p->last_percent) {
    std::cout << "\r[git] downloading: " << percent << "% ("
              << stats->received_objects << "/" << stats->total_objects << " objects)"
              << std::flush;
    if (p) p->last_percent = percent;
  }
  return 0;
}

static bool clone_git_remote_worktree(const std::string& remote_spec,
                                      const std::string& token,
                                      const std::filesystem::path& out_worktree,
                                      std::string* err) {
  auto init_rc = git_libgit2_init();
  if (init_rc < 0) {
    if (err) *err = "libgit2 init failed";
    return false;
  }

  git_repository* repo = nullptr;
  git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
  GitAuthPayload auth{token};
  GitProgressPayload progress;
  opts.fetch_opts.callbacks.credentials = git_credential_cb;
  opts.fetch_opts.callbacks.payload = &auth;
  opts.fetch_opts.callbacks.transfer_progress = git_transfer_progress_cb;
  opts.fetch_opts.callbacks.payload = &progress;

  std::cout << "Cloning Git remote: " << remote_spec << "\n";

  auto rc = git_clone(&repo, remote_spec.c_str(), out_worktree.string().c_str(), &opts);
  if (repo) git_repository_free(repo);
  if (rc != 0) {
    std::cout << "\n";
    const git_error* ge = git_error_last();
    if (err) *err = ge && ge->message ? ge->message : "git clone failed";
    git_libgit2_shutdown();
    return false;
  }

  std::cout << "\r[git] downloading: 100% (complete)                              \n";
  git_libgit2_shutdown();
  return true;
}

int clone(const forge_cli::ParsedArgs& a) {
  if (a.positionals.empty()) {
    std::cerr << "forge clone: provide a Forge/Git path or URL\n";
    return 2;
  }
  const auto remote_spec = a.positionals[0];
  auto remote_path = std::filesystem::path(remote_spec);
  auto dest = a.positionals.size() >= 2 ? std::filesystem::path(a.positionals[1]) : remote_path.filename();
  auto parsed = forge_core::remote::parse_url(remote_spec);
  if ((dest.empty() || dest == ".") && parsed) {
    auto p = std::filesystem::path(parsed->path);
    auto name = p.filename().string();
    if (name.empty()) name = p.parent_path().filename().string();
    if (name.empty()) name = "repo";
    dest = name;
  }
  if (dest.empty()) dest = "repo";
  if (std::filesystem::exists(dest)) {
    std::cerr << "forge clone: destination already exists\n";
    return 1;
  }

  std::string err;
  auto local_forge = std::filesystem::exists(remote_path / ".forge") ||
                     (std::filesystem::exists(remote_path) && remote_path.filename() == ".forge");
  auto local_git = std::filesystem::exists(remote_path / ".git");
  auto looks_like_git_remote = infer_git_remote(remote_spec);

  if (local_git) {
    std::cout << "Importing local Git repository into Forge...\n";
    if (!import_git_worktree_as_forge(remote_path, dest, remote_spec, &err)) {
      std::cerr << "forge clone: " << err << "\n";
      return 1;
    }
    std::cout << "Clone completed.\n";
    return 0;
  }

  if (local_forge) {
    auto source_forge_dir = (remote_path.filename() == ".forge") ? remote_path : (remote_path / ".forge");
    std::cout << "Cloning local Forge repository...\n";
    if (!copy_dir(source_forge_dir, dest / ".forge", &err)) {
      std::cerr << "forge clone: " << err << "\n";
      return 1;
    }
  } else if (parsed && (parsed->scheme == forge_core::remote::Scheme::Http || parsed->scheme == forge_core::remote::Scheme::Https) &&
             !looks_like_git_remote) {
    std::cout << "Cloning Forge remote: " << remote_spec << "\n";
    std::filesystem::create_directories(dest);
    if (!forge_core::repo::init(dest, &err)) {
      std::cerr << "forge clone: " << err << "\n";
      return 1;
    }
    forge_core::remote::RemoteSpec spec;
    spec.url = remote_spec;
    spec.remote_name = "origin";
    spec.token = a.option("token");
    auto fr = forge_core::remote::fetch_into_repo(dest, spec);
    if (!fr.ok) {
      std::cerr << "forge clone: " << fr.err << "\n";
      return 1;
    }
    std::cout << "Forge remote downloaded.\n";
  } else {
    auto tmp = std::filesystem::temp_directory_path() /
               ("forge-git-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    if (!clone_git_remote_worktree(remote_spec, a.option("token"), tmp, &err)) {
      std::cerr << "forge clone: " << err << "\n";
      return 1;
    }
    auto ok = import_git_worktree_as_forge(tmp, dest, remote_spec, &err);
    std::error_code ec;
    std::filesystem::remove_all(tmp, ec);
    if (!ok) {
      std::cerr << "forge clone: " << err << "\n";
      return 1;
    }
    std::cout << "Clone completed.\n";
    return 0;
  }

  if (!forge_platform::fs::ensure_dir(dest / ".forge" / "remotes")) {
    std::cerr << "forge clone: failed to create remotes dir\n";
    return 1;
  }
  if (!forge_platform::fs::write_file_atomic(dest / ".forge" / "remotes" / "origin", remote_spec + "\n")) {
    std::cerr << "forge clone: failed to write origin remote\n";
    return 1;
  }

  auto head_commit = forge_core::refs::resolve_head_commit(dest, &err);
  if (head_commit && !head_commit->empty()) {
    if (!forge_core::checkout::checkout_commit(dest, *head_commit, &err)) {
      std::cerr << "forge clone: " << err << "\n";
      return 1;
    }
  }
  std::cout << "Clone completed.\n";
  return 0;
}

}

