#pragma once

#include "game_value.h"
#include "prime1/CGameState.hpp"
#include "prime1/CSimplePool.hpp"

class CGameGlobalObjects : public game_value<> {
public:
    static constexpr uint32_t LOCATION = 0x80457798;
    static constexpr uint32_t FRAMECOUNT_LOCATION = 0x805A93C0;
    CGameGlobalObjects(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    CSimplePool mainPool = CSimplePool(ptr(), 0xCC);
    game_ptr<CGameState> gameState = game_ptr<CGameState>(ptr(), 0x134);
    game_u32 frameCount = game_u32(FRAMECOUNT_LOCATION, 0x00);
};
