#pragma once

#include "game_value.h"
#include "prime1/actors/CPlayer.hpp"
#include "prime1/CWorld.hpp"
#include "prime1/CPlayerState.hpp"
#include "prime1/CCameraManager.hpp"
#include "prime1/CObjectList.hpp"
#include "prime1/CRandom16.hpp"

class CStateManager : public game_value<> {
public:
    static constexpr uint32_t LOCATION = 0x8045A1A8;
    CStateManager(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

//    game_ptr<CObjectList> allObjs = game_ptr<CObjectList>(ptr(), 0x80C);
    game_ptr<CObjectList> allObjs = game_ptr<CObjectList>(ptr(), 0x810);
    game_ptr<CPlayer> player = game_ptr<CPlayer>(ptr(), 0x84C);
    game_ptr<CWorld> world = game_ptr<CWorld>(ptr(), 0x850);
    game_ptr<CCameraManager> cameraManager = game_ptr<CCameraManager>(ptr(), 0x870);
    game_rc_ptr<CPlayerState> playerState = game_rc_ptr<CPlayerState>(ptr(), 0x8b8);

    game_u32 nextAreaID = game_u32(ptr(), 0x8CC);
    game_u32 prevAreaID = game_u32(ptr(), 0x8D0);
    CRandom16 random = CRandom16(ptr(), 0x8FC);
};
