#pragma once

#include <cstdint>
#include <vector>

namespace MemoryAccess {
  std::vector<int> getDolphinPids();
  bool attachToProcess(int pid);
  void dolphin_memcpy(void *dest, std::size_t offset, std::size_t size);

  int getAttachedPid();

  uint32_t beToHost32(uint32_t bigEndian);
  uint32_t hostToBe32(uint32_t value);
  uint64_t beToHost64(uint64_t bigEndian);
  uint64_t hostToBe64(uint64_t value);
}