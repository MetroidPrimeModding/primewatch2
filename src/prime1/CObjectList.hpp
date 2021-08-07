#pragma once

#include "game_value.h"
#include "prime1/actors/CEntity.hpp"

class SObjectListEntry : public game_value<8> {
public:
    SObjectListEntry(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_u32 entityPtr = game_u32(ptr(), 0x0);
    game_ptr<CEntity> entity = game_ptr<CEntity>(ptr(), 0x0);
    game_u16 next = game_u16(ptr(), 0x4);
    game_u16 prev = game_u16(ptr(), 0x6);
};

class CObjectList : public game_value<> {
public:
    CObjectList(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_array<SObjectListEntry, 1024> list = game_array<SObjectListEntry, 1024>(ptr(), 0x4);
    game_u32 typeEnum = game_u32(ptr(), 0x2004);
    game_u16 firstID = game_u16(ptr(), 0x2008);
    game_u16 count = game_u16(ptr(), 0x200A);
};
