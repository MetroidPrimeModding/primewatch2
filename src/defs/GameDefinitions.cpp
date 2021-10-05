#include "GameDefinitions.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "json_optional.hpp"

using namespace nlohmann;
using namespace std;

namespace GameDefinitions {
  unordered_map<string, GameEnum> enums_by_name;
  unordered_map<string, GameStruct> structs_by_name;

  void loadEnumsFromJsonList(const json &enums);

  void loadStructsFromJsonList(const json &structs);

  void loadDefinitionsFromPath(const char *path) {
    json rawJson;

    {
      std::ifstream instream(path);
      instream >> rawJson;
    }

    // load enums
    loadEnumsFromJsonList(rawJson["enums"]);
    loadStructsFromJsonList(rawJson["structs"]);
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

      for (auto jsonMember : jsonStruct["members"]) {
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
}