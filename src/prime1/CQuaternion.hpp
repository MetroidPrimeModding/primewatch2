#pragma once

#include "game_value.h"

class CQuaternion : public game_value<0x4 * 4> {
public:
    CQuaternion(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_array<game_float, 0x4> values = game_array<game_float, 0x4>(ptr(), 0x0);
    game_array<game_float, 0x4> rawValues = game_array<game_float, 0x4>(ptr(), 0x0);
    game_float x = game_float(ptr(), 0x0);
    game_float y = game_float(ptr(), 0x4);
    game_float z = game_float(ptr(), 0x8);
    game_float w = game_float(ptr(), 0xC);
};
