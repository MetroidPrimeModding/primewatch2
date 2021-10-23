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

    std::optional<GameEnumValue> valueByName(const std::string& subName) const;
    GameEnumValue valueByValue(uint32_t value) const;
  };

  struct GameMember {
    std::string name;
    std::string type;
    uint32_t offset;
    std::optional<uint32_t> bit{};
    std::optional<uint32_t> bitLength{};
    std::optional<uint32_t> arrayLength{};
    bool pointer{false};

    GameMember operator [](std::string subName) const;
    std::optional<GameMember> memberByName(const std::string& subName) const;

    bool read_bool() const;
    uint8_t read_u8() const;
    uint16_t read_u16() const;
    uint32_t read_u32() const;
    uint64_t read_u64() const;
    float read_f32() const;
    double read_f64() const;

    std::string read_string() const;
    bool extendsClass(const std::string &className) const;
  };

  struct GameStruct {
    std::string name;
    uint32_t size;
    std::optional<uint32_t> vtable;
    std::vector<std::string> extends;
    std::vector<GameMember> members_by_order;
    std::map<std::string, GameMember> members_by_name;
    std::map<uint32_t, GameMember> members_by_offset;

    std::optional<GameMember> memberByName(const std::string& subName) const;
    bool extendsClass(const std::string &className);
  };

  void loadDefinitionsFromPath(const char *path);

  std::optional<GameEnum> enumByName(const std::string& name);
  std::optional<GameStruct> structByName(const std::string& name);

//  extern std::unordered_map<std::string, GameEnum> enums_by_name;
//  extern  std::unordered_map<std::string, GameStruct> structs_by_name;
};


