#pragma once

#include "game_value.h"

class SObjectTag : public game_value<0x8> {
public:
  SObjectTag(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value<0x8>(base_ptr, ptr_offset) {}

  game_u32 fourCC = game_u32(ptr(), 0x0);
  game_u32 id = game_u32(ptr(), 0x4);
};
