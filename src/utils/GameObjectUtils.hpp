#pragma once

#include "defs/GameDefinitions.hpp"

namespace GameObjectUtils {
  GameDefinitions::GameMember getObjectByEntityID(uint16_t eid);
  std::vector<GameDefinitions::GameMember> getAllObjects();
  std::vector<GameDefinitions::GameMember> getAllCObjectReferences();
  std::vector<GameDefinitions::GameMember> getAllLoadingDatas();

  std::string objectTagToString(GameDefinitions::GameMember sObjectTag);
  std::string fourCCToString(uint32_t cc);
};


