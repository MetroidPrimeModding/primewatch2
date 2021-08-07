#pragma once

#include "game_value.h"
#include "prime1/CHealthInfo.hpp"
#include "prime1/CReservedVector.hpp"

class CGameState : public game_value<> {
public:
    CGameState(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_u32 mlvlID = game_u32(ptr(), 0x84);
    game_double playTime = game_double(ptr(), 0xa0);
};

