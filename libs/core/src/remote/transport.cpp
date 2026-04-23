#include "forge_core/remote/transport.h"

#include "forge_core/remote/url.h"
#include "forge_core/transfer/frames.h"
#include "forge_core/transfer/snapshot.h"
#include "forge_core/crypto/credentials.h"
#include "forge_platform/fs.h"

#include <httplib.h>

#include <memory>

namespace forge_core::remote {

static std::optional<std::string> read_remote_file(const std::filesystem::path& wd, const std::string& name) {
  return forge_platform::fs::read_file(wd / ".forge" / "remotes" / name);
}

static std::optional<std::string> read_default_token() {
  const char* t = std::getenv("FORGE_TOKEN");
  if (!t) return std::nullopt;
  return std::string(t);
}

static std::string resolve_token(const std::filesystem::path& workdir, const RemoteSpec& spec) {
  if (!spec.token.empty()) return spec.token;
  if (!spec.remote_name.empty()) {
    if (auto t = forge_core::crypto::credentials::get_token(workdir, spec.remote_name, nullptr)) return *t;
  }
  return read_default_token().value_or("");
}

static FetchResult fetch_http(const std::filesystem::path& workdir, const RemoteUrl& u, const RemoteSpec& spec) {
  std::unique_ptr<httplib::Client> cli;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  if (u.scheme == Scheme::Https) cli = std::make_unique<httplib::SSLClient>(u.host, u.port);
  else
#endif
    cli = std::make_unique<httplib::Client>(u.host, u.port);
  cli->set_read_timeout(60, 0);
  cli->set_write_timeout(60, 0);
  httplib::Headers headers;
  auto token = resolve_token(workdir, spec);
  if (!token.empty()) headers.emplace("Authorization", "Bearer " + token);

  auto res = cli->Post((u.path + "/fetch").c_str(), headers, "", "application/octet-stream");
  if (!res) return {false, "http request failed"};
  if (res->status != 200) return {false, "http status " + std::to_string(res->status)};

  std::string err;
  auto decoded = forge_core::transfer::decode_frames(res->body, &err);
  if (!decoded) return {false, "decode failed"};
  if (!forge_core::transfer::apply_snapshot(workdir, *decoded, &err)) return {false, err};
  return {true, ""};
}

static PushResult push_http(const std::filesystem::path& workdir, const RemoteUrl& u, const RemoteSpec& spec) {
  std::unique_ptr<httplib::Client> cli;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  if (u.scheme == Scheme::Https) cli = std::make_unique<httplib::SSLClient>(u.host, u.port);
  else
#endif
    cli = std::make_unique<httplib::Client>(u.host, u.port);
  cli->set_read_timeout(60, 0);
  cli->set_write_timeout(60, 0);
  httplib::Headers headers;
  auto token = resolve_token(workdir, spec);
  if (!token.empty()) headers.emplace("Authorization", "Bearer " + token);

  std::string err;
  auto entries = forge_core::transfer::snapshot_forge_dir(workdir, &err);
  if (!err.empty()) return {false, err};
  auto body = forge_core::transfer::encode_frames(entries);

  auto res = cli->Post((u.path + "/push").c_str(), headers, body, "application/octet-stream");
  if (!res) return {false, "http request failed"};
  if (res->status != 200) return {false, "http status " + std::to_string(res->status)};
  return {true, ""};
}

FetchResult fetch_into_repo(const std::filesystem::path& workdir, const RemoteSpec& remote) {
  auto u = parse_url(remote.url);
  if (!u) return {false, "invalid url"};
  if (u->scheme == Scheme::Http || u->scheme == Scheme::Https) return fetch_http(workdir, *u, remote);
  return {false, "transport not implemented"};
}

PushResult push_from_repo(const std::filesystem::path& workdir, const RemoteSpec& remote) {
  auto u = parse_url(remote.url);
  if (!u) return {false, "invalid url"};
  if (u->scheme == Scheme::Http || u->scheme == Scheme::Https) return push_http(workdir, *u, remote);
  return {false, "transport not implemented"};
}

}

