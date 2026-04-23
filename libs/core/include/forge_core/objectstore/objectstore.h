#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace forge_core::objectstore {

enum class ObjectType : unsigned {
  Blob = 1,
  Tree = 2,
  Commit = 3,
  Tag = 4
};

struct StoredObject {
  ObjectType type;
  std::string data;
};

std::string type_name(ObjectType t);
ObjectType parse_type(std::string_view s);

struct ObjectId {
  std::string hex;
};

ObjectId store_loose(const std::filesystem::path& workdir, ObjectType type, std::string_view bytes, std::string* err);
std::optional<StoredObject> load_loose(const std::filesystem::path& workdir, std::string_view hex, std::string* err);

}

