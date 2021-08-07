#ifndef DOLPHIN_EMU_CENTITY_H
#define DOLPHIN_EMU_CENTITY_H

#include <prime1/rstl/rstl.hpp>
#include "game_value.h"

class CEntity : public game_value<> {
public:
    static constexpr uint8_t ACTIVE_MASK = 0x80;
    static constexpr uint8_t GRAVEYARD_MASK = 0x40;
    static constexpr uint8_t BLOCKED_MASK = 0x20;
    static constexpr uint8_t USE_MASK = 0x10;

    CEntity(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_u32 areaID = game_u32(ptr(), 0x4);
    game_u16 uniqueID = game_u16(ptr(), 0x8);
    game_u32 editorID = game_u32(ptr(), 0xC);
    rstl::string<char> name = rstl::string<char>(ptr(), 0x10);
    game_u8 status = game_u8(ptr(), 0x30);
};


#endif //DOLPHIN_EMU_CENTITY_H
