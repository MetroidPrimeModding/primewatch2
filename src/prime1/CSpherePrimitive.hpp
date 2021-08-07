#ifndef DOLPHIN_EMU_CSPHEREPRIMITIVE_H
#define DOLPHIN_EMU_CSPHEREPRIMITIVE_H

#include "game_value.h"
#include "CVector3f.hpp"

class CSpherePrimitive : public game_value<> {
public:
    CSpherePrimitive(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    CVector3f origin = CVector3f(ptr(), 0x10);
    game_float radius = game_float(ptr(), 0x1C);
};


#endif //DOLPHIN_EMU_CTRANSFORM_H
