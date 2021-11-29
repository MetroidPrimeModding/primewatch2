#include "GameDefinitions.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <GameMemory.h>
#include <fmt/format.h>
#include "json_optional.hpp"
#include <bstruct.hpp>

using namespace nlohmann;
using namespace std;

namespace GameDefinitions {
  unordered_map<string, GameEnum> enums_by_name;
  unordered_map<string, GameStruct> structs_by_name;
  string bstruct_error;

  void loadEnumsFromJsonList(const json &enums);

  void loadStructsFromJsonList(const json &structs);

  bool loadDefinitionsFromPath(const char *path) {
    json rawJson;

    enums_by_name.clear();
    structs_by_name.clear();

    CompileResult result = compile_glob_to_json(path);
    if (result.err) {
      bstruct_error = string(result.value);
      cerr << result.value << endl;
      return false;
    }

    {
      rawJson = json::parse(result.value);
    }

    // load enums
    loadEnumsFromJsonList(rawJson["enums"]);
    loadStructsFromJsonList(rawJson["structs"]);

    release_result(result);

    return isLoaded();
  }

  std::optional<GameEnum> enumByName(const std::string &name) {
    if (enums_by_name.count(name)) {
      return enums_by_name[name];
    } else {
      return {};
    }
  }

  std::optional<GameStruct> structByName(const std::string &name) {
    if (structs_by_name.count(name)) {
      return structs_by_name[name];
    } else {
      return {};
    }
  }

  bool isLoaded() {
    return !enums_by_name.empty() && !structs_by_name.empty();
  }

  std::string getError() {
    return bstruct_error;
  }

  void loadEnumsFromJsonList(const json &enums) {
    for (auto jsonEnum: enums) {
      map<uint32_t, GameEnumValue> values_by_value;
      map<string, GameEnumValue> values_by_name;

      for (auto value: jsonEnum["values"]) {
        GameEnumValue v{
            .name = value["name"].get<string>(),
            .value = value["value"].get<uint32_t>()
        };
        values_by_value[v.value] = v;
        values_by_name[v.name] = v;
      }

      GameEnum e{
          .name = jsonEnum["name"].get<string>(),
          .size = jsonEnum["size"].get<uint32_t>(),
          .values_by_value = values_by_value,
          .values_by_name = values_by_name,
      };

      enums_by_name[e.name] = e;
    }

    cout << "Loaded " << enums_by_name.size() << " enums" << endl;
  }

  void loadStructsFromJsonList(const json &structs) {
    for (auto jsonStruct: structs) {
      vector<GameMember> members_by_order;
      map<string, GameMember> members_by_name;
      map<uint32_t, GameMember> members_by_offset;

      for (auto jsonMember: jsonStruct["members"]) {
        GameMember m{
            .name=jsonMember["name"].get<string>(),
            .type=jsonMember["type"].get<string>(),
            .offset=jsonMember["offset"].get<uint32_t>(),
            .bit=jsonMember["bit"].get<optional<uint32_t>>(),
            .bitLength=jsonMember["bitLength"].get<optional<uint32_t>>(),
            .arrayLength=jsonMember["arrayLength"].get<optional<uint32_t>>(),
            .pointer=jsonMember["pointer"].get<optional<bool>>().value_or(false),
        };
        members_by_order.push_back(m);
        members_by_offset[m.offset] = m;
        members_by_name[m.name] = m;
      }

      GameStruct s{
          .name = jsonStruct["name"].get<string>(),
          .size = jsonStruct["size"].get<uint32_t>(),
          .vtable = jsonStruct["vtable"].get<optional<uint32_t>>(),
          .extends = jsonStruct["extends"].get<optional<vector<string>>>().value_or(vector<string>()),
          .members_by_order = members_by_order,
          .members_by_name = members_by_name,
          .members_by_offset = members_by_offset,
      };
      structs_by_name[s.name] = s;
    }

    cout << "Loaded " << structs_by_name.size() << " structs" << endl;
  }


  optional<GameEnumValue> GameEnum::valueByName(const string &subName) const {
    if (values_by_name.count(subName)) {
      return values_by_name.at(subName);
    } else {
      return {};
    }
  }

  GameEnumValue GameEnum::valueByValue(uint32_t value) const {
    if (values_by_value.count(value)) {
      return values_by_value.at(value);
    } else {
      return {
          .name = fmt::format("Unknown ({})", value),
          .value = value
      };
    }
  }

  optional<GameMember> GameStruct::memberByName(const string &subName) const {
    if (members_by_name.count(subName)) {
      return members_by_name.at(subName);
    } else {
      for (auto &extend: extends) {
        auto parent = GameDefinitions::structByName(extend);
        if (parent.has_value()) {
          auto res = parent->memberByName(subName);
          if (res.has_value()) {
            return res;
          }
        }
      }
      return {};
    }
  }

  bool GameStruct::extendsClass(const string &className) {
    for (auto &extend: extends) {
      if (extend == className) {
        return true;
      }
      auto parent = GameDefinitions::structByName(extend);
      if (parent.has_value()) {
        if (parent->extendsClass(className)) {
          return true;
        }
      }
    }
    return false;
  }

  std::optional<GameMember> GameMember::memberByName(const std::string &subName) const {
    auto myType = structByName(this->type);
    if (!myType.has_value()) return {};
    auto subMember = myType->memberByName(subName);
    if (!subMember.has_value()) return {};

    GameMember res = *subMember;
    res.offset = this->offset + subMember->offset;
    if (subMember->pointer) {
      res.offset = GameMemory::read_u32(res.offset);
    }
    return res;
  }

  bool GameMember::extendsClass(const string &className) const {
    if (this->type == className) return true;
    auto myType = structByName(this->type);
    if (!myType.has_value()) return false;
    return myType.value().extendsClass(className);
  }

  template<typename T>
  T getBits(T v, optional<uint32_t> bitOptional, optional<uint32_t> lengthOptional) {
    uint32_t bit = bitOptional.value_or(0);
    uint32_t length = lengthOptional.value_or(0);
    T mask = (1ULL << length) - 1;
    if (length == 0) {
      mask = ~0;
    }
    return (v >> bit) & mask;
  }


  bool GameMember::read_bool() const {
    return read_u8() != 0;
  }

  uint8_t GameMember::read_u8() const {
    return getBits<>(GameMemory::read_u8(offset), bit, bitLength);
  }

  uint16_t GameMember::read_u16() const {
    return getBits<>(GameMemory::read_u16(offset), bit, bitLength);
  }

  uint32_t GameMember::read_u32() const {
    return getBits<>(GameMemory::read_u32(offset), bit, bitLength);
  }

  uint64_t GameMember::read_u64() const {
    return getBits<>(GameMemory::read_u64(offset), bit, bitLength);
  }

  float GameMember::read_f32() const {
    return GameMemory::read_float(offset);
  }

  double GameMember::read_f64() const {
    return GameMemory::read_double(offset);
  }

  std::string GameMember::read_string() const {
    constexpr int maxLen = 255;
    string val;
    for (int i = 0; i < maxLen; i++) {
      char c = static_cast<char>(GameMemory::read_u8(offset + i));
      if (c == 0) break;
      val += c;
    }
    return val;
  }

  // WARNING: this will crash if it doesn't exit. I'm so safe :)
  GameMember GameMember::operator[](string subName) const {
    optional<GameMember> mbr = memberByName(subName);
    if (!mbr.has_value()) throw std::invalid_argument(fmt::format("Unknown member {} {}.{}", type, name, subName));
    return *mbr;
  }
}