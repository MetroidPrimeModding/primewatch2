#ifndef DOLPHIN_EMU_CMATRIX4F_HPP
#define DOLPHIN_EMU_CMATRIX4F_HPP

#include "game_value.h"
#include "json.hpp"

class CMatrix4f : public game_value<0x4 * 16> {
public:
    CMatrix4f(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_array<game_float, 16> values = game_array<game_float, 16>(ptr(), 0x0);
    game_array<game_u32, 16> rawValues = game_array<game_u32, 16>(ptr(), 0x0);
};

#endif //DOLPHIN_EMU_CMATRIX4F_HPP
