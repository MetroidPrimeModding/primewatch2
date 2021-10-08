#include <array>
#include <memory>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <fmt/format.h>

#include "GameMemory.h"
#include "MemoryAccess.hpp"

using namespace std;

namespace GameMemory {
  array<char, MemoryAccess::DOLPHIN_MEMORY_SIZE> memory{0};

  void updateFromDolphin() {
    if (MemoryAccess::getAttachedPid() > 0) {
      MemoryAccess::dolphin_memcpy(memory.data(), 0, memory.size());
    }
  }

  void loadFromPath(const string& path) {
    ifstream in = ifstream(path);
    in.read(memory.data(), MemoryAccess::DOLPHIN_MEMORY_SIZE);
    fmt::print("Read {:x} bytes", in.gcount());
  }

  uint32_t getRealPtr(uint32_t address) {
    uint32_t masked = address & 0x7FFFFFFFu;
    if (masked > MemoryAccess::DOLPHIN_MEMORY_SIZE) {
      return 0;
    }
    return masked;
  }

  std::uint8_t read_u8(std::uint32_t address) {
    uint8_t res;
    memcpy(&res, memory.data() + getRealPtr(address), sizeof(res));
    return res;
  }

  std::uint16_t read_u16(std::uint32_t address) {
    uint16_t res;
    memcpy(&res, memory.data() + getRealPtr(address), sizeof(res));
    return MemoryAccess::beToHost16(res);
  }

  std::uint32_t read_u32(std::uint32_t address) {
    uint32_t res;
    memcpy(&res, memory.data() + getRealPtr(address), sizeof(res));
    return MemoryAccess::beToHost32(res);
  }

  std::uint64_t read_u64(std::uint32_t address) {
    uint64_t res;
    memcpy(&res, memory.data() + getRealPtr(address), sizeof(res));
    return MemoryAccess::beToHost64(res);
  }

  float read_float(std::uint32_t address) {
    uint32_t value = read_u32(address);
    return *reinterpret_cast<float *>(&value);
  }

  double read_double(std::uint32_t address) {
    uint64_t value = read_u64(address);
    return *reinterpret_cast<double *>(&value);
  }
}
