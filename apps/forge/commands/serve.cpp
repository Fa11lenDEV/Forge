#include "forge_cli/args.h"
#include "forge_core/transfer/frames.h"
#include "forge_core/transfer/snapshot.h"
#include "forge_server/handlers.h"

#include <httplib.h>

#include <filesystem>
#include <iostream>

namespace forge_app::commands {

static std::filesystem::path opt_repo(const forge_cli::ParsedArgs& a) {
  auto r = a.option("repo");
  if (!r.empty()) return std::filesystem::path(r);
  return std::filesystem::current_path();
}

static bool token_ok(const httplib::Request& req) {
  const char* need = std::getenv("FORGE_TOKEN");
  if (!need || std::string(need).empty()) return true;
  auto it = req.headers.find("Authorization");
  if (it == req.headers.end()) return false;
  auto v = it->second;
  auto p = std::string("Bearer ");
  if (v.rfind(p, 0) != 0) return false;
  return v.substr(p.size()) == need;
}

static int serve_http(const forge_cli::ParsedArgs& a) {
  auto addr = a.option("http");
  if (addr.empty()) addr = a.option("listen");
  if (addr.empty()) {
    std::cerr << "forge serve: provide --http=:8080\n";
    return 2;
  }

  std::string host = (a.has_flag("public") ? "0.0.0.0" : "127.0.0.1");
  int port = 8080;
  auto colon = addr.rfind(':');
  if (colon != std::string::npos) {
    if (colon > 0) host = addr.substr(0, colon);
    port = std::stoi(addr.substr(colon + 1));
  } else {
    port = std::stoi(addr);
  }

  auto repo = opt_repo(a);
  httplib::Server srv;
  srv.set_payload_max_length(256ull * 1024ull * 1024ull);

  srv.Get(R"(/info/refs)", [&](const httplib::Request& req, httplib::Response& res) {
    if (!token_ok(req)) { res.status = 401; return; }
    std::string err;
    auto r = forge_server::info_refs(repo, &err);
    if (!err.empty()) { res.status = 500; res.set_content(err, "text/plain"); return; }
    res.set_content(r.text, "text/plain");
  });

  srv.Post(R"(/fetch)", [&](const httplib::Request& req, httplib::Response& res) {
    if (!token_ok(req)) { res.status = 401; return; }
    std::string err;
    auto entries = forge_core::transfer::snapshot_forge_dir(repo, &err);
    if (!err.empty()) { res.status = 500; res.set_content(err, "text/plain"); return; }
    auto body = forge_core::transfer::encode_frames(entries);
    res.set_content(std::move(body), "application/octet-stream");
  });

  srv.Post(R"(/push)", [&](const httplib::Request& req, httplib::Response& res) {
    if (!token_ok(req)) { res.status = 401; return; }
    std::string err;
    auto decoded = forge_core::transfer::decode_frames(req.body, &err);
    if (!decoded) { res.status = 400; res.set_content("decode failed", "text/plain"); return; }
    if (!forge_core::transfer::apply_snapshot(repo, *decoded, &err)) {
      res.status = 500;
      res.set_content(err, "text/plain");
      return;
    }
    res.set_content("OK\n", "text/plain");
  });

  std::cout << "Serving " << repo.string() << " on http://" << host << ":" << port << "\n";
  if (!srv.listen(host, port)) {
    std::cerr << "forge serve: listen failed\n";
    return 1;
  }
  return 0;
}

static int serve_https(const forge_cli::ParsedArgs& a) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  auto addr = a.option("https");
  if (addr.empty()) {
    std::cerr << "forge serve: provide --https=:8443 --cert=... --key=...\n";
    return 2;
  }
  auto cert = a.option("cert");
  auto key = a.option("key");
  if (cert.empty() || key.empty()) {
    std::cerr << "forge serve: provide --cert and --key\n";
    return 2;
  }

  std::string host = (a.has_flag("public") ? "0.0.0.0" : "127.0.0.1");
  int port = 8443;
  auto colon = addr.rfind(':');
  if (colon != std::string::npos) {
    if (colon > 0) host = addr.substr(0, colon);
    port = std::stoi(addr.substr(colon + 1));
  } else {
    port = std::stoi(addr);
  }

  auto repo = opt_repo(a);
  httplib::SSLServer srv(cert.c_str(), key.c_str());
  srv.set_payload_max_length(256ull * 1024ull * 1024ull);

  srv.Get(R"(/info/refs)", [&](const httplib::Request& req, httplib::Response& res) {
    if (!token_ok(req)) { res.status = 401; return; }
    std::string err;
    auto r = forge_server::info_refs(repo, &err);
    if (!err.empty()) { res.status = 500; res.set_content(err, "text/plain"); return; }
    res.set_content(r.text, "text/plain");
  });

  srv.Post(R"(/fetch)", [&](const httplib::Request& req, httplib::Response& res) {
    if (!token_ok(req)) { res.status = 401; return; }
    std::string err;
    auto entries = forge_core::transfer::snapshot_forge_dir(repo, &err);
    if (!err.empty()) { res.status = 500; res.set_content(err, "text/plain"); return; }
    auto body = forge_core::transfer::encode_frames(entries);
    res.set_content(std::move(body), "application/octet-stream");
  });

  srv.Post(R"(/push)", [&](const httplib::Request& req, httplib::Response& res) {
    if (!token_ok(req)) { res.status = 401; return; }
    std::string err;
    auto decoded = forge_core::transfer::decode_frames(req.body, &err);
    if (!decoded) { res.status = 400; res.set_content("decode failed", "text/plain"); return; }
    if (!forge_core::transfer::apply_snapshot(repo, *decoded, &err)) {
      res.status = 500;
      res.set_content(err, "text/plain");
      return;
    }
    res.set_content("OK\n", "text/plain");
  });

  std::cout << "Serving " << repo.string() << " on https://" << host << ":" << port << "\n";
  if (!srv.listen(host, port)) {
    std::cerr << "forge serve: listen failed\n";
    return 1;
  }
  return 0;
#else
  std::cerr << "forge serve: built without OpenSSL support\n";
  return 1;
#endif
}

static int serve_stdio(const forge_cli::ParsedArgs& a) {
  auto repo = opt_repo(a);
  const char* need = std::getenv("FORGE_TOKEN");
  if (need && std::string(need).size() > 0) {
    std::string auth;
    if (!std::getline(std::cin, auth)) return 1;
    const std::string p = "AUTH ";
    if (auth.rfind(p, 0) != 0 || auth.substr(p.size()) != need) return 1;
  }

  std::string cmd;
  if (!std::getline(std::cin, cmd)) return 1;
  if (cmd == "FETCH") {
    std::string err;
    auto entries = forge_core::transfer::snapshot_forge_dir(repo, &err);
    if (!err.empty()) return 1;
    auto body = forge_core::transfer::encode_frames(entries);
    std::cout.write(body.data(), static_cast<std::streamsize>(body.size()));
    return 0;
  }
  if (cmd == "PUSH") {
    std::string payload((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
    std::string err;
    auto decoded = forge_core::transfer::decode_frames(payload, &err);
    if (!decoded) return 1;
    if (!forge_core::transfer::apply_snapshot(repo, *decoded, &err)) return 1;
    std::cout << "OK\n";
    return 0;
  }
  return 2;
}

int serve(const forge_cli::ParsedArgs& a) {
  if (a.options.find("stdio") != a.options.end()) return serve_stdio(a);
  if (!a.option("https").empty()) return serve_https(a);
  if (!a.option("http").empty() || !a.option("listen").empty()) return serve_http(a);
  std::cerr << "forge serve: use --http=:8080 or --stdio\n";
  return 2;
}

}

