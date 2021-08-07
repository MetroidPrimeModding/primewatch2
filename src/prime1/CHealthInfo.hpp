#pragma once

#include "game_value.h"

class CHealthInfo : public game_value<> {
public:
  CHealthInfo(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

  game_float health = game_float(ptr(), 0x0);
  game_float knockbackResistance = game_float(ptr(), 0x4);
};

