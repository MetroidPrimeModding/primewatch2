#pragma once

#include "game_value.h"
#include "prime1/CSpherePrimitive.hpp"

class CMorphBall : public game_value<> {
public:
    CMorphBall(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    CSpherePrimitive collisionPrimitive = CSpherePrimitive(ptr(), 0x38);
};
