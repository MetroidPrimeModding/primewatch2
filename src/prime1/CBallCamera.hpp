#ifndef DOLPHIN_EMU_CBALLCAMERA_HPP
#define DOLPHIN_EMU_CBALLCAMERA_HPP

#include "game_value.h"
#include "prime1/actors/CGameCamera.hpp"

class CBallCamera : public CGameCamera {
public:
    CBallCamera(uint32_t base_ptr, uint32_t ptr_offset = 0) : CGameCamera(base_ptr, ptr_offset) {}


};

#endif //DOLPHIN_EMU_CBALLCAMERA_HPP
