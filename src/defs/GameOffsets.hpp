#pragma once

#include <cstdint>
#include "defs/GameDefinitions.hpp"

const GameDefinitions::GameMember g_stateManager{.name="g_stateManager", .type="CStateManager", .offset=0x8045A1A8};
const GameDefinitions::GameMember g_main{.name="g_main", .type="CMain", .offset=0x80457560};
const GameDefinitions::GameMember gp_MemoryCard{.name="gp_MemoryCard", .type="CMemoryCardSys", .offset=0x805a8c44, .pointer = true};
const GameDefinitions::GameMember gp_TweakPlayer{.name="gp_TweakPlayer", .type="CTweakPlayer", .offset=0x805a8cd8, .pointer = true};
