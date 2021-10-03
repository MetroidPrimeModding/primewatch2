#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <optional>

namespace GameDefinitions {
  struct GameEnumValue {
    std::string name;
    uint32_t value;
  };

  struct GameEnum {
    std::string name;
    uint32_t size;
    std::map<uint32_t, GameEnumValue> values_by_value;
    std::map<std::string, GameEnumValue> values_by_name;
  };

  struct GameMember {
    std::string name;
    std::string type;
    uint32_t offset;
    std::optional<uint32_t> bit;
    std::optional<uint32_t> bitLength;
    std::optional<uint32_t> arrayLength;
    bool pointer;
  };

  struct GameStruct {
    std::string name;
    uint32_t size;
    std::optional<uint32_t> vtable;
    std::vector<std::string> extends;
    std::map<std::string, GameMember> members_by_name;
    std::map<uint32_t, GameMember> members_by_offset;
  };

  void loadDefinitionsFromPath(const char *path);

  extern std::unordered_map<std::string, GameEnum> enums_by_name;
  extern  std::unordered_map<std::string, GameStruct> structs_by_name;
};


