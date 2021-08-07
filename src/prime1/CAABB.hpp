#pragma once

#include "game_value.h"
#include "CVector3f.hpp"

class CAABB : public game_value<0x4 * 3 * 2> {
public:
  CAABB(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

  game_array<game_float, 0x6> values = game_array<game_float, 0x6>(ptr(), 0x0);
  game_array<game_u32, 0x6> rawValues = game_array<game_u32, 0x6>(ptr(), 0x0);
  CVector3f min = CVector3f(ptr(), 0x0);
  CVector3f max = CVector3f(ptr(), 0xC);
};
