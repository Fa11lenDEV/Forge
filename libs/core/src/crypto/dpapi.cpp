#include "forge_core/crypto/dpapi.h"

#ifdef _WIN32
#  include <windows.h>
#  include <wincrypt.h>
#   pragma comment(lib, "Crypt32.lib")
#endif

namespace forge_core::crypto::dpapi {

std::optional<std::vector<std::uint8_t>> protect(const std::vector<std::uint8_t>& plaintext, std::string* err) {
#ifdef _WIN32
  DATA_BLOB in{};
  in.pbData = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(plaintext.data()));
  in.cbData = static_cast<DWORD>(plaintext.size());
  DATA_BLOB out{};

  if (!CryptProtectData(&in, L"forge", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out)) {
    if (err) *err = "CryptProtectData failed";
    return std::nullopt;
  }

  std::vector<std::uint8_t> v(out.pbData, out.pbData + out.cbData);
  LocalFree(out.pbData);
  return v;
#else
  if (err) *err = "dpapi not supported";
  return std::nullopt;
#endif
}

std::optional<std::vector<std::uint8_t>> unprotect(const std::vector<std::uint8_t>& blob, std::string* err) {
#ifdef _WIN32
  DATA_BLOB in{};
  in.pbData = const_cast<BYTE*>(reinterpret_cast<const BYTE*>(blob.data()));
  in.cbData = static_cast<DWORD>(blob.size());
  DATA_BLOB out{};

  if (!CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out)) {
    if (err) *err = "CryptUnprotectData failed";
    return std::nullopt;
  }

  std::vector<std::uint8_t> v(out.pbData, out.pbData + out.cbData);
  LocalFree(out.pbData);
  return v;
#else
  if (err) *err = "dpapi not supported";
  return std::nullopt;
#endif
}

}

