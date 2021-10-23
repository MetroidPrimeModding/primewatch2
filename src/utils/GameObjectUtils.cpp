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

  vector<GameMember> getAllObjects() {
    GameMember globalList = g_stateManager["allObjects"];
    uint16_t first = globalList["firstID"].read_u16();
    uint16_t size = globalList["size"].read_u16();
    if (size > 1024) size = 1024; // failsafe too

    GameMember entry = globalList["list"];
    GameStruct type = *GameDefinitions::structByName(entry.type);

    vector<GameMember> allObjects;
    allObjects.reserve(size);

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

      allObjects.push_back(entity);

      currentId = currentEntry["next"].read_u16();
    }

    return allObjects;
  }

};
