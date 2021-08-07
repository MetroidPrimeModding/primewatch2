#pragma once

#include "game_value.h"

template<class T, uint32_t capacity>
class CReservedVector : public game_value<capacity * T::size + 4> {
public:
  CReservedVector(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value<capacity * T::size + 4>(base_ptr,
                                                                                                   ptr_offset) {}

  game_u32 size = game_u32(this->ptr(), 0x0);
  game_array<T, capacity> array = game_array<T, capacity>(this->ptr(), 0x4);
};
