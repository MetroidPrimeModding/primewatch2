#ifndef DOLPHIN_EMU_CPLAYERCAMERABOB_HPP
#define DOLPHIN_EMU_CPLAYERCAMERABOB_HPP

#include "game_value.h"

class CPlayerCameraBob : public game_value<> {
public:
    CPlayerCameraBob(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    CTransform cameraBobTransform = CTransform(ptr(), 0x2C);
    game_float bobMagnitude = game_float(ptr(), 0x10);
    game_float bobTimeScale = game_float(ptr(), 0x18);
    game_u32 rawBobMagnitude = game_u32(ptr(), 0x10);
    game_u32 rawBobTimeScale = game_u32(ptr(), 0x18);
};

#endif //DOLPHIN_EMU_CPLAYERCAMERABOB_HPP
