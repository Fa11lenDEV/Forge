add_library(forge_core STATIC
  src/repo/repo.cpp
  src/objectstore/objectstore.cpp
  src/index/index.cpp
  src/refs/refs.cpp
  src/commit/commit.cpp
  src/diff/diff.cpp
  src/checkout/checkout.cpp
  src/merge/merge.cpp
  src/rebase/rebase.cpp
  src/tag/tag.cpp
  src/stash/stash.cpp
  src/submodule/submodule.cpp
  src/remote/url.cpp
  src/remote/transport.cpp
  src/transfer/frames.cpp
  src/transfer/snapshot.cpp
)

target_include_directories(forge_core PUBLIC include)
target_link_libraries(forge_core PUBLIC forge_platform forge_format)

