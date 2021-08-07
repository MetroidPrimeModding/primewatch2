#ifndef DOLPHIN_EMU_CFIRSTPERSONCAMERA_HPP
#define DOLPHIN_EMU_CFIRSTPERSONCAMERA_HPP

#include "game_value.h"
#include "prime1/actors/CGameCamera.hpp"

class CFirstPersonCamera : public CGameCamera {
public:
    CFirstPersonCamera(uint32_t base_ptr, uint32_t ptr_offset = 0) : CGameCamera(base_ptr, ptr_offset) {}

    CTransform gunFollowXf = CTransform(ptr(), 0x190);
};


#endif //DOLPHIN_EMU_CFIRSTPERSONCAMERA_HPP
