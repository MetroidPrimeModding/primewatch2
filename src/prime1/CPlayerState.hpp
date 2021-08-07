#pragma once

#include "game_value.h"
#include "prime1/CHealthInfo.hpp"
#include "prime1/CReservedVector.hpp"

class CPlayerState : public game_value<> {
public:
  CPlayerState(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

  game_u32 currentBeam = game_u32(ptr(), 0x8);
  CHealthInfo healthInfo = CHealthInfo(ptr(), 0xC); //2AC
  game_u32 currentVisor = game_u32(ptr(), 0x14);
  game_u32 currentSuit = game_u32(ptr(), 0x20);
  CReservedVector<game_u32, 41> powerups = CReservedVector<game_u32, 41>(ptr(), 0x24);
};
