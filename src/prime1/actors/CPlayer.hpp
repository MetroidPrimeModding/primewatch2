#ifndef DOLPHIN_EMU_CPLAYER_H
#define DOLPHIN_EMU_CPLAYER_H

#include "prime1/actors/CPhysicsActor.hpp"
#include "prime1/actors/CPlayerGun.hpp"
#include "prime1/actors/CMorphBall.hpp"
#include "prime1/CPlayerCameraBob.hpp"

class CPlayer : public CPhysicsActor {
public:
    CPlayer(uint32_t base_ptr, uint32_t ptr_offset = 0) : CPhysicsActor(base_ptr, ptr_offset) {}

    game_u8 jumpState = game_u8(ptr(), 0x258);
    game_float sjTimer = game_float(ptr(), 0x28C);
    game_u32 cameraState = game_u32(ptr(), 0x2f4);
    game_u32 morphState = game_u32(ptr(), 0x2f8);

    game_ptr<CPlayerGun> gun = game_ptr<CPlayerGun>(ptr(), 0x490);
    game_ptr<CMorphBall> morphBall = game_ptr<CMorphBall>(ptr(), 0x768);
    game_ptr<CPlayerCameraBob> cameraBob = game_ptr<CPlayerCameraBob>(ptr(), 0x76C);
};


#endif //DOLPHIN_EMU_CPLAYER_H
