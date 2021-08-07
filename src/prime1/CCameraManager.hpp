#ifndef DOLPHIN_EMU_CCAMERAMANAGER_HPP
#define DOLPHIN_EMU_CCAMERAMANAGER_HPP

#include "game_value.h"
#include "CFirstPersonCamera.hpp"
#include "CBallCamera.hpp"

class CCameraManager : public game_value<> {
public:
    CCameraManager(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_ptr<CFirstPersonCamera> firstPerson = game_ptr<CFirstPersonCamera>(ptr(), 0x7C);
    game_ptr<CBallCamera> ball = game_ptr<CBallCamera>(ptr(), 0x80);
};

#endif //DOLPHIN_EMU_CCAMERAMANAGER_HPP
