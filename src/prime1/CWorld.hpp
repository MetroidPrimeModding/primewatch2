#pragma once

#include "game_value.h"
#include "CGameArea.hpp"

class CWorld : public game_value<> {
public:
  CWorld(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

  game_u32 phase = game_u32(ptr(), 0x4);
  game_u32 mlvlID = game_u32(ptr(), 0x8);
  game_u32 strgID = game_u32(ptr(), 0xC);
  rstl::vector<rstl::auto_ptr<CGameArea>> areas = rstl::vector<rstl::auto_ptr<CGameArea>>(ptr(), 0x18);
  game_u32 relays = game_u32(ptr(), 0x2C);
  game_u32 currentAreaID = game_u32(ptr(), 0x68);
};
