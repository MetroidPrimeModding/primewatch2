#pragma once

#include "game_value.h"

class CPlayerGun : public game_value<> {
public:
    CPlayerGun(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}


};
