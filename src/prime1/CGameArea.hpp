#pragma once

#include "game_value.h"

enum class EPhase {
  LoadHeader,
  LoadSecSizes,
  ReserveSections,
  LoadDataSections,
  WaitForFinish
};

enum class EChain {
  Invalid = -1,
  ToDeallocate,
  Deallocated,
  Loading,
  Alive,
  AliveJudgement
};

enum class EOcclusionState {
  Occluded,
  Visible
};

class CPostConstructed;

class CGameArea : public game_value<> {
public:
  CGameArea(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

  game_u32 selfIDX = game_u32(ptr(), 0x4);
  game_u32 nameStrg = game_u32(ptr(), 0x4);
  game_u32 mrea = game_u32(ptr(), 0x84);
  game_u32 areaID = game_u32(ptr(), 0x88);
  game_u32_bit<24> postConstructedBool = game_u32_bit<24>(ptr(), 0xF0);
  game_u32_bit<25> active = game_u32_bit<25>(ptr(), 0xF0);
  game_u32_bit<26> tokensReady = game_u32_bit<26>(ptr(), 0xF0);
  game_u32_bit<27> loadPaused = game_u32_bit<27>(ptr(), 0xF0);
  game_u32_bit<28> validated = game_u32_bit<28>(ptr(), 0xF0);
  game_u32_enum<EPhase> phase = game_u32_enum<EPhase>(ptr(), 0xF4);
  game_u32 loadTransactions = game_u32(ptr(), 0xF8);
  game_u32_enum<EChain> curChain = game_u32_enum<EChain>(ptr(), 0x138);
  game_ptr<CPostConstructed> postConstructed = game_ptr<CPostConstructed>(ptr(), 0x12C);
};

class CPostConstructed : public game_value<> {
public:
  CPostConstructed(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

  game_u32_enum<EOcclusionState> occlusionState = game_u32_enum<EOcclusionState>(ptr(), 0x10DC);
};
