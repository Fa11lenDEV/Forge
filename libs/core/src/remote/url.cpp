#include "forge_core/remote/url.h"

#include <cctype>

namespace forge_core::remote {

static bool starts_with(const std::string& s, const std::string& p) {
  return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

std::string normalize_path(const std::string& s) {
  if (s.empty()) return "/";
  if (s[0] == '/') return s;
  return "/" + s;
}

static std::optional<RemoteUrl> parse_http_like(const std::string& s, Scheme scheme) {
  auto pos = s.find("://");
  if (pos == std::string::npos) return std::nullopt;
  auto rest = s.substr(pos + 3);

  RemoteUrl u;
  u.scheme = scheme;
  u.raw = s;

  auto slash = rest.find('/');
  auto authority = slash == std::string::npos ? rest : rest.substr(0, slash);
  u.path = slash == std::string::npos ? "/" : normalize_path(rest.substr(slash));

  auto at = authority.find('@');
  auto hostport = authority;
  if (at != std::string::npos) {
    u.user = authority.substr(0, at);
    hostport = authority.substr(at + 1);
  }

  auto colon = hostport.rfind(':');
  if (colon != std::string::npos && colon + 1 < hostport.size()) {
    u.host = hostport.substr(0, colon);
    auto ps = hostport.substr(colon + 1);
    int port = 0;
    for (char c : ps) {
      if (c < '0' || c > '9') return std::nullopt;
      port = port * 10 + (c - '0');
      if (port > 65535) return std::nullopt;
    }
    if (port <= 0) return std::nullopt;
    u.port = port;
  } else {
    u.host = hostport;
    u.port = (scheme == Scheme::Https) ? 443 : 80;
  }
  return u;
}

static std::optional<RemoteUrl> parse_ssh(const std::string& s) {
  if (starts_with(s, "ssh://")) return parse_http_like(s, Scheme::Ssh);

  auto at = s.find('@');
  auto colon = s.find(':');
  if (at == std::string::npos || colon == std::string::npos || colon < at) return std::nullopt;

  RemoteUrl u;
  u.scheme = Scheme::Ssh;
  u.raw = s;
  u.user = s.substr(0, at);
  u.host = s.substr(at + 1, colon - (at + 1));
  u.port = 22;
  u.path = normalize_path(s.substr(colon + 1));
  return u;
}

std::optional<RemoteUrl> parse_url(const std::string& s) {
  if (starts_with(s, "http://")) return parse_http_like(s, Scheme::Http);
  if (starts_with(s, "https://")) return parse_http_like(s, Scheme::Https);
  if (starts_with(s, "ssh://")) return parse_http_like(s, Scheme::Ssh);
  if (auto ssh = parse_ssh(s)) return ssh;

  RemoteUrl u;
  u.scheme = Scheme::File;
  u.raw = s;
  u.path = s;
  return u;
}

}

