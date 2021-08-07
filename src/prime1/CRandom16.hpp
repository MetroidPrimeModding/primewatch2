#pragma once

#include "game_value.h"

class CRandom16 : public game_value<> {
public:
  CRandom16(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

  game_u32 seed = game_u32(ptr(), 0x0);
};
