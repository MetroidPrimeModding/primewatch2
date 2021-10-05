#include "MemoryAccess.hpp"

#include <filesystem>
#include <iostream>

#ifdef __linux__

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

#endif

using namespace std;
namespace fs = filesystem;

namespace MemoryAccess {
#ifdef __linux__
  int attachedPid = -1;
  uint8_t *emuRAMAddressStart = nullptr;

  vector<int> getDolphinPids() {
    vector<int> res{};
    string path = "/proc";
    for (const auto &entry : fs::directory_iterator(path)) {
      if (entry.is_directory()) {
        const auto &pidPath = entry.path();
        int pid = 0;
        try {
          pid = stoi(pidPath.filename().string());
        } catch (const invalid_argument &e) {
          continue;
        }

        const auto exeLinkPath = pidPath / "exe";
        fs::path exePath;
        try {
          if (!fs::exists(exeLinkPath)) continue;
          exePath = fs::read_symlink(exeLinkPath);
        } catch (fs::filesystem_error &e) {
          continue;
        }

        if (exePath.filename() == "dolphin-emu") {
          printf("pid: %d path %s\n", pid, exePath.c_str());
          res.push_back(pid);
        } else {
          continue;
        }
      }
    }

    return res;
  }

  bool attachToProcess(int pid) {
    constexpr size_t size = 0x2040000;

    if (emuRAMAddressStart != nullptr) {
      cout << "Closing old shared memory" << endl;
      if (munmap(emuRAMAddressStart, size) < 0) {
        cerr << "Failed to close old memory" << endl;
        exit(4);
      }
      emuRAMAddressStart = nullptr;
      attachedPid = -1;
    }

    cout << "Dolphin found, PID " << pid << endl;
    const string file_name = "/dolphin-emu." + to_string(pid);
    int fd = shm_open(file_name.c_str(), O_RDWR, 0600);

    if (fd < 0) {
      cerr << "Failed to open shared memory" << endl;
      return false;
    }

    cout << "Opened shmem" << endl;

    void *mem = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
      cerr << "failed to map shared memory" << endl;
      close(fd);
      return false;
    }

    emuRAMAddressStart = reinterpret_cast<uint8_t *>(mem);
    attachedPid = pid;
    close(fd);
    return true;
  }

  int getAttachedPid() {
    return attachedPid;
  }

  uint32_t getRealPtr(uint32_t address) {
    uint32_t masked = address & 0x7FFFFFFFu;
    if (masked > DOLPHIN_MEMORY_SIZE) {
      return 0;
    }
    return masked;
  }

  void dolphin_memcpy(void *dest, size_t offset, size_t size) {
    if (emuRAMAddressStart == nullptr) {
      return;
    }
    if (size > DOLPHIN_MEMORY_SIZE) {
      size = DOLPHIN_MEMORY_SIZE;
    }
    memcpy(dest, emuRAMAddressStart + getRealPtr(offset), size);
  }

  uint32_t beToHost16(uint32_t bigEndian) {
    return be16toh(bigEndian);
  }

  uint32_t hostToBe16(uint32_t value) {
    return htobe16(value);
  }

  uint32_t beToHost32(uint32_t bigEndian) {
    return be32toh(bigEndian);
  }

  uint32_t hostToBe32(uint32_t value) {
    return htobe32(value);
  }

  uint64_t beToHost64(uint64_t bigEndian) {
    return be64toh(bigEndian);
  }

  uint64_t hostToBe64(uint64_t value) {
    return htobe64(value);
  }

#else

#error Not yet implemented for other platforms

#endif
}