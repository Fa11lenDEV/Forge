#include "forge_core/rebase/rebase.h"

#include "forge_core/checkout/checkout.h"
#include "forge_core/refs/refs.h"

namespace forge_core::rebase {

bool rebase_onto(const std::filesystem::path& workdir, const std::string& upstream_branch, bool hard, std::string* err) {
  auto head = forge_core::refs::read_head(workdir, err);
  if (!head || !head->is_ref) {
    if (err) *err = "detached HEAD not supported yet";
    return false;
  }

  auto up = forge_core::refs::read_ref(workdir, "refs/heads/" + upstream_branch, err);
  if (!up || up->empty()) {
    if (err) *err = "upstream not found";
    return false;
  }

  if (!forge_core::refs::write_ref(workdir, head->value, *up, err)) return false;
  if (hard) return forge_core::checkout::checkout_commit(workdir, *up, err);
  return true;
}

}

