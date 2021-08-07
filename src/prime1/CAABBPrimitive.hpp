#pragma once

#include "game_value.h"
#include "prime1/CAABB.hpp"

class CAABBPrimitive : public game_value<> {
public:
    CAABBPrimitive(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    CAABB aabb = CAABB(ptr(), 0x10);
};

