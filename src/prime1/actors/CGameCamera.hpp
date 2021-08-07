#ifndef DOLPHIN_EMU_CGAMECAMERA_HPP
#define DOLPHIN_EMU_CGAMECAMERA_HPP

#include "CActor.hpp"
#include "prime1/CVector3f.hpp"
#include "prime1/CQuaternion.hpp"
#include "prime1/CMatrix4f.hpp"
#include "prime1/CTransform.hpp"
#include "prime1/CAABBPrimitive.hpp"

class CGameCamera : public CActor {
public:
    CGameCamera(uint32_t base_ptr, uint32_t ptr_offset = 0) : CActor(base_ptr, ptr_offset) {}

    game_u32 watchedObject = game_u32(ptr(), 0xE8);
    CMatrix4f perspectiveMatrix = CMatrix4f(ptr(), 0xEC);
    CTransform camTransform = CTransform(ptr(), 0x12C);
    game_float currentFov = game_float(ptr(), 0x15C);
    game_float znear = game_float(ptr(), 0x160);
    game_float zfar = game_float(ptr(), 0x164);
    game_float aspect = game_float(ptr(), 0x168);
    game_float fov = game_float(ptr(), 0x184);
};

#endif //DOLPHIN_EMU_CGAMECAMERA_HPP
