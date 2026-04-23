#include "forge_core/commit/commit.h"
#include "forge_core/index/index.h"
#include "forge_core/remote/transport.h"
#include "forge_core/repo/repo.h"
#include "forge_core/refs/refs.h"
#include "forge_core/transfer/frames.h"
#include "forge_core/transfer/snapshot.h"

#include <httplib.h>

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

static void write_file(const std::filesystem::path& p, const std::string& s) {
  std::error_code ec;
  std::filesystem::create_directories(p.parent_path(), ec);
  std::ofstream f(p, std::ios::binary | std::ios::trunc);
  f << s;
}

static std::string read_text(const std::filesystem::path& p) {
  std::ifstream f(p, std::ios::binary);
  std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
  return s;
}

static int pick_port() { return 18080; }

int main() {
  auto base = std::filesystem::temp_directory_path() / "forge_remote_http";
  std::error_code ec;
  std::filesystem::remove_all(base, ec);
  std::filesystem::create_directories(base, ec);

  auto repoA = base / "A";
  auto repoB = base / "B";
  std::filesystem::create_directories(repoA, ec);
  std::filesystem::create_directories(repoB, ec);

  std::string err;
  assert(forge_core::repo::init(repoA, &err));
  write_file(repoA / "a.txt", "hello\n");
  assert(forge_core::index::add_paths(repoA, {"a.txt"}, &err));
  auto c1 = forge_core::commit::create_commit_from_index(repoA, "c1", &err);
  assert(!c1.commit_hex.empty());

  httplib::Server srv;
  srv.Post("/fetch", [&](const httplib::Request&, httplib::Response& res) {
    std::string e;
    auto entries = forge_core::transfer::snapshot_forge_dir(repoA, &e);
    auto body = forge_core::transfer::encode_frames(entries);
    res.set_content(std::move(body), "application/octet-stream");
  });
  srv.Post("/push", [&](const httplib::Request& req, httplib::Response& res) {
    std::string e;
    auto decoded = forge_core::transfer::decode_frames(req.body, &e);
    assert(decoded.has_value());
    assert(forge_core::transfer::apply_snapshot(repoA, *decoded, &e));
    res.set_content("OK\n", "text/plain");
  });

  auto port = pick_port();
  std::thread t([&] { srv.listen("127.0.0.1", port); });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  assert(forge_core::repo::init(repoB, &err));
  forge_core::remote::RemoteSpec spec;
  spec.url = "http://127.0.0.1:" + std::to_string(port);

  auto f = forge_core::remote::fetch_into_repo(repoB, spec);
  assert(f.ok);

  auto a_head_before = read_text(repoA / ".forge" / "refs" / "heads" / "main");

  write_file(repoB / "b.txt", "world\n");
  assert(forge_core::index::add_paths(repoB, {"b.txt"}, &err));
  auto c2 = forge_core::commit::create_commit_from_index(repoB, "c2", &err);
  assert(!c2.commit_hex.empty());

  auto p = forge_core::remote::push_from_repo(repoB, spec);
  assert(p.ok);

  auto a_head_after = read_text(repoA / ".forge" / "refs" / "heads" / "main");
  assert(!a_head_after.empty());

  srv.stop();
  t.join();

  assert(a_head_before != a_head_after);
  return 0;
}

