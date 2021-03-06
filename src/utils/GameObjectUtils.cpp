#include "GameObjectUtils.hpp"
#include "defs/GameOffsets.hpp"
#include "defs/GameVtables.hpp"

#include <fmt/format.h>


using namespace GameDefinitions;
using namespace std;

namespace GameObjectUtils {

  GameMember getObjectByEntityID(uint16_t eid) {
    uint16_t actualId = eid & 0x3FF;
    GameMember globalList = g_stateManager["allObjects"];


    GameMember entry = globalList["list"];
    GameStruct type = *GameDefinitions::structByName(entry.type);
    entry.offset += type.size * actualId;

    GameMember result = entry["entity"];
    return result;
  }

  map<TUniqueID, GameMember> getAllObjects() {
    GameMember globalList = g_stateManager["allObjects"];
    uint16_t first = globalList["firstID"].read_u16();
    uint16_t size = globalList["size"].read_u16();
    if (size > 1024) size = 1024; // failsafe too

    GameMember entry = globalList["list"];
    GameStruct type = *GameDefinitions::structByName(entry.type);

    map<TUniqueID, GameMember> allObjects;

    int count = 0;
    uint16_t currentId = first;
    while (currentId != 0xFFFF) {
      // emergency exit in case of bad timing
      if (count > size) break;
      count++;

      GameMember currentEntry = entry;
      currentEntry.offset += type.size * currentId;
      GameMember entity = currentEntry["entity"];
      uint32_t vtable = entity["vtable"].read_u32();
      if (MP1_VTABLES.count(vtable)) {
        string vtableType = MP1_VTABLES[vtable];
        if (GameDefinitions::structByName(vtableType).has_value()) {
          entity.type = vtableType;
        }
      }

      allObjects[currentId] = entity;

      currentId = currentEntry["next"].read_u16();
    }

    return allObjects;
  }

  void inOrderTraversal(vector<GameMember> &res, const GameMember &member, int depth);

  vector<GameMember> getAllCObjectReferences() {
    GameMember mainPool = g_main["globalObjects"]["mainPool"];
    GameMember tree = mainPool["resources"];

    vector<GameMember> res;
    unsigned long size = tree["size"].read_u32();
    res.reserve(size);
    inOrderTraversal(res, tree["root"], 0);
    return res;
  }

  void inOrderTraversal(vector<GameMember> &res, const GameMember &member, int size) {
    if (member.offset == 0) return;
    if (res.size() > size) return; // emergency exit!

    inOrderTraversal(res, member["left"], size);

    GameMember value = member["data"]["b"];
    res.push_back(value);

    inOrderTraversal(res, member["right"], size);
  };

  string objectTagToString(GameDefinitions::GameMember sObjectTag) {
    string res;
    res.reserve(9);
    res += fmt::format("{:08x}", sObjectTag["id"].read_u32());
    uint32_t fourCC = sObjectTag["fourCC"].read_u32();
    res += ".";
    res += fourCCToString(fourCC);
    return res;
  }

  string fourCCToString(uint32_t cc) {
    string res = "    ";
    for (int i = 0; i < 4; i++) {
      char c = static_cast<char>(cc >> ((3 - i) * 8));
      res[i] = c;
    }
    return res;
  }

  vector<GameMember> getAllLoadingDatas() {

    GameMember resFactory = g_main["globalObjects"]["gameResFactory"];

    GameMember list = resFactory["loadList"];

    uint32_t size = list["size"].read_u32();
    vector<GameMember> res;
    res.reserve(size);

    GameMember current = list["first"];
    GameMember end = list["end"];
    while (current.offset != end.offset && current.offset != 0){
      // Emergency exit
      if (res.size() > size) break;
      res.push_back(current["item"]);
      current = current["next"];
    };

    return res;
  }
};
