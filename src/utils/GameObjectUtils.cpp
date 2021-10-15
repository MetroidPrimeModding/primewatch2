#include "GameObjectUtils.hpp"
#include "defs/GameOffsets.hpp"

#include <fmt/format.h>

using namespace GameDefinitions;

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

};
