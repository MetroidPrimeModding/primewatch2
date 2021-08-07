#pragma once

#include "game_value.h"

class CVector3f : public game_value<0x4 * 3> {
public:
  CVector3f(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

  game_array<game_float, 0x3> values = game_array<game_float, 0x3>(ptr(), 0x0);
  game_array<game_u32, 0x3> rawValues = game_array<game_u32, 0x3>(ptr(), 0x0);
  game_float x = game_float(ptr(), 0x0);
  game_float y = game_float(ptr(), 0x4);
  game_float z = game_float(ptr(), 0x8);
};

