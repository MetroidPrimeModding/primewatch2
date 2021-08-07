#include <array>
#include <memory>
#include <chrono>
#include "GameMemory.h"

#include "../MemoryAccess.hpp"

using namespace std;


namespace GameMemory {
  unique_ptr<array<char, MemoryAccess::DOLPHIN_MEMORY_SIZE>> memory;

  void updateFromDolphin() {
    if (MemoryAccess::getAttachedPid() > 0) {
      if (memory == nullptr) {
        memory = make_unique<array<char, MemoryAccess::DOLPHIN_MEMORY_SIZE>>();
      }
      MemoryAccess::dolphin_memcpy(memory->data(), 0, memory->size());
    }
  }


  std::uint32_t read_u32(std::uint32_t address) {
    uint32_t res;
    MemoryAccess::dolphin_memcpy(&res, address, sizeof(res));
    return res;
  }

  std::uint64_t read_u64(std::uint32_t address) {
    uint64_t res;
    MemoryAccess::dolphin_memcpy(&res, address, sizeof(res));
    return res;
  }

  float read_float(std::uint32_t address) {
    uint32_t value = read_u32(address);
    return *reinterpret_cast<float*>(&value);
  }

  double read_double(std::uint32_t address) {
    uint64_t value = read_u64(address);
    return *reinterpret_cast<double*>(&value);
  }
}
