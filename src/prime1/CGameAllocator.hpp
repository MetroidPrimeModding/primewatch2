//
// Created by pwootage on 1/4/17.
//

#include "game_value.h"

#ifndef DOLPHIN_EMU_CGAMEALLOCATOR_H
#define DOLPHIN_EMU_CGAMEALLOCATOR_H

class CMemoryBlock : public game_value<> {
public:
    CMemoryBlock(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_u32 sentinel = game_u32(ptr(), 0x0);
    game_u32 size = game_u32(ptr(), 0x4);
    game_u32 fileAndLinePtr = game_u32(ptr(), 0x8);
    game_u32 typePtr = game_u32(ptr(), 0xC);
    game_ptr<CMemoryBlock> prev = game_ptr<CMemoryBlock>(ptr(), 0x10);
    game_ptr<CMemoryBlock> next = game_ptr<CMemoryBlock>(ptr(), 0x14);
    game_u32 dataStart = game_u32(ptr(), 0x18);
    game_u32 canary = game_u32(ptr(), 0x1C);
};

class CGameAllocator : public game_value<> {
public:
    static constexpr uint32_t LOCATION = 0x804BFD64;
    CGameAllocator(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_u32 heapSize = game_u32(ptr(), 0x8);
    game_ptr<CMemoryBlock> first = game_ptr<CMemoryBlock>(ptr(), 0xC);
    game_ptr<CMemoryBlock> last = game_ptr<CMemoryBlock>(ptr(), 0x10);
};

#endif //DOLPHIN_EMU_CGAMEALLOCATOR_H
