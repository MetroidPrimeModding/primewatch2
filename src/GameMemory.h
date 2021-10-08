#pragma once

#include <cstdint>
#include <string>
#include <array>
#include "MemoryAccess.hpp"

namespace GameMemory {
  extern std::array<char, MemoryAccess::DOLPHIN_MEMORY_SIZE> memory;

  void updateFromDolphin();
  void loadFromPath(const std::string& path);

  std::uint8_t read_u8(std::uint32_t address);
  std::uint16_t read_u16(std::uint32_t address);
  std::uint32_t read_u32(std::uint32_t address);
  std::uint64_t read_u64(std::uint32_t address);
  float read_float(std::uint32_t address);
  double read_double(std::uint32_t address);
}

