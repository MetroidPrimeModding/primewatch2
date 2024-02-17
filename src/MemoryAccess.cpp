#include "MemoryAccess.hpp"

#include <filesystem>
#include <iostream>

#if defined(__linux__)

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

#elif defined(__APPLE__)

#include <libkern/OSByteOrder.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#import <sys/proc_info.h>
#import <libproc.h>

#elif defined(WIN32)

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#endif

using namespace std;
namespace fs = filesystem;

namespace MemoryAccess {
#if defined(__linux__)
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

    detachFromProcess();

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

  void detachFromProcess() {
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
#elif defined(WIN32)
int attachedPid = -1;
HANDLE dolphinProcHandle = 0;
uint64_t emuRAMAddressStart = 0;
bool MEM2Present = false;

// Shamelessly ripped from Dolphin-memory-engine
// https://github.com/aldelaro5/Dolphin-memory-engine/blob/master/Source/DolphinProcess/Windows/WindowsDolphinProcess.cpp
// Returns true if we found and set emuRAMAddressStart
bool getEmuRAMAddressStart() {
    if (!dolphinProcHandle)
        return false;

    MEMORY_BASIC_INFORMATION info;
    bool MEM1Found = false;
    for (unsigned char* p = nullptr; VirtualQueryEx(dolphinProcHandle, p, &info, sizeof(info)) == sizeof(info); p += info.RegionSize) {
        if (MEM1Found) {
            uint64_t regionBaseAddress = 0;
            memcpy(&regionBaseAddress, &(info.BaseAddress), sizeof(info.BaseAddress));
            if (regionBaseAddress == emuRAMAddressStart + 0x10000000) {
                // View the comment for MEM1
                PSAPI_WORKING_SET_EX_INFORMATION wsInfo;
                wsInfo.VirtualAddress = info.BaseAddress;
                if (QueryWorkingSetEx(dolphinProcHandle, &wsInfo, sizeof(PSAPI_WORKING_SET_EX_INFORMATION))) {
                    if (wsInfo.VirtualAttributes.Valid)
                        MEM2Present = true;
                }

                break;
            }
            else if (regionBaseAddress > emuRAMAddressStart + 0x10000000) {
                MEM2Present = false;
                break;
            }

            continue;
        }

        if (info.RegionSize == 0x2000000 && info.Type == MEM_MAPPED) {
            // Here, it's likely the right page, but it can happen that multiple pages with these criteria
            // exists and have nothing to do with the emulated memory. Only the right page has valid
            // working set information so an additional check is required that it is backed by physical memory.
            PSAPI_WORKING_SET_EX_INFORMATION wsInfo;
            wsInfo.VirtualAddress = info.BaseAddress;
            if (QueryWorkingSetEx(dolphinProcHandle, &wsInfo, sizeof(PSAPI_WORKING_SET_EX_INFORMATION))) {
                if (wsInfo.VirtualAttributes.Valid) {
                    memcpy(&emuRAMAddressStart, &(info.BaseAddress), sizeof(info.BaseAddress));
                    MEM1Found = true;
                    cout << "Found ram start: " << emuRAMAddressStart << endl;
                }
            }
        }
    }

    if (!emuRAMAddressStart)
        return false; // Dolphin is running, but the emulation hasn't started

    return true;
}

vector<int> getDolphinPids() {
    vector<int> res{};
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    bool found = false;
    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (strncmp(entry.szExeFile, "Dolphin.exe", 10) == 0) {
                res.push_back(entry.th32ProcessID);
            }
        }
    }

    CloseHandle(snapshot);

    return res;
}

bool attachToProcess(int pid) {
    constexpr size_t size = 0x2040000;

    detachFromProcess();

    cout << "Connectiong to Dolphin pid " << pid << endl;
    dolphinProcHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ, FALSE, pid);

    if (!getEmuRAMAddressStart()) {
        // Wait for dolphin to start running a game
        cout << "Detected dolphin isn't running a game. We'll check for it in copy." << endl;
    }

    attachedPid = pid;

    return true;
}

void detachFromProcess() {
    constexpr size_t size = 0x2040000;
    if (dolphinProcHandle != nullptr) {
        cout << "Closing process" << endl;
        CloseHandle(dolphinProcHandle);
        dolphinProcHandle = nullptr;
        emuRAMAddressStart = 0;
        attachedPid = -1;
    }
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

void dolphin_memcpy(void* dest, size_t offset, size_t size) {
    if (emuRAMAddressStart == 0) {
        if (dolphinProcHandle) {
            if (!getEmuRAMAddressStart()) {
                return;
            }
        }
        return;
    }
    if (size > DOLPHIN_MEMORY_SIZE) {
        size = DOLPHIN_MEMORY_SIZE;
    }
    SIZE_T read = 0;
    if (ReadProcessMemory(dolphinProcHandle, reinterpret_cast<void*>(emuRAMAddressStart + getRealPtr(offset)), dest, size, &read) == 0) {
        int err = GetLastError();
        cerr << "Failed to read memory from" << offset << ". Error: " << err << endl;
        if (err == 299) {
            cerr << "Game probablly closed. Will continue looking." << endl;
            emuRAMAddressStart = 0;
        }
    }
    if (read != size) {
        cerr << "Failed to read enough from " << hex << offset << ". Read: " << read << " of " << size << dec << endl;
    }
}

// assume windows is little endian
uint32_t beToHost16(uint32_t value) {
    return 
        ((value & 0xFF00) >> 8) |
        ((value & 0x00FF) << 8);
}

uint32_t hostToBe16(uint32_t value) {
    return 
        ((value & 0xFF00) >> 8) |
        ((value & 0x00FF) << 8);
}

uint32_t beToHost32(uint32_t value) {
    return
        ((value & 0xFF000000) >> 24) |
        ((value & 0x00FF0000) >> 8) |
        ((value & 0x0000FF00) << 8) | 
        ((value & 0x000000FF) << 24);
}

uint32_t hostToBe32(uint32_t value) {
    return
        ((value & 0xFF000000) >> 24) |
        ((value & 0x00FF0000) >> 8) |
        ((value & 0x0000FF00) << 8) |
        ((value & 0x000000FF) << 24);
}

uint64_t beToHost64(uint64_t value) {
    return
        ((value & 0xFF00000000000000L) >> 56) |
        ((value & 0x00FF000000000000L) >> 40) | 
        ((value & 0x0000FF0000000000L) >> 24) |
        ((value & 0x000000FF00000000L) >> 8) |
        ((value & 0x00000000FF000000L) << 8) |
        ((value & 0x0000000000FF0000L) << 24) |
        ((value & 0x000000000000FF00L) << 40) |
        ((value & 0x00000000000000FFL) << 56);
}

uint64_t hostToBe64(uint64_t value) {
    return
        ((value & 0xFF00000000000000L) >> 56) |
        ((value & 0x00FF000000000000L) >> 40) |
        ((value & 0x0000FF0000000000L) >> 24) |
        ((value & 0x000000FF00000000L) >> 8) |
        ((value & 0x00000000FF000000L) << 8) |
        ((value & 0x0000000000FF0000L) << 24) |
        ((value & 0x000000000000FF00L) << 40) |
        ((value & 0x00000000000000FFL) << 56);
}

#elif defined(__APPLE__)
  int attachedPid = -1;
  uint8_t *emuRAMAddressStart = nullptr;

  vector<int> getDolphinPids() {
    vector<int> res{};

    int numberOfProcesses = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    pid_t pids[numberOfProcesses];
    bzero(pids, sizeof(pids));
    proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
    for (int i = 0; i < numberOfProcesses; ++i) {
      if (pids[i] == 0) { continue; }
      char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
      bzero(pathBuffer, PROC_PIDPATHINFO_MAXSIZE);
      proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer));
      if (strlen(pathBuffer) > 0) {
        printf("path: %s\n", pathBuffer);
        string exePath = pathBuffer;
        if (exePath.ends_with("Dolphin")) {
          printf("pid: %d path %s\n", pids[i], exePath.c_str());
          res.push_back(pids[i]);
        }
      }
    }

    return res;
  }


  bool attachToProcess(int pid) {
    constexpr size_t size = 0x2040000;

    detachFromProcess();

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

  void detachFromProcess() {
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
    return OSSwapBigToHostInt16(bigEndian);
  }

  uint32_t hostToBe16(uint32_t value) {
    return OSSwapHostToBigInt16(value);
  }

  uint32_t beToHost32(uint32_t bigEndian) {
    return OSSwapBigToHostInt32(bigEndian);
  }

  uint32_t hostToBe32(uint32_t value) {
    return OSSwapHostToBigInt32(value);
  }

  uint64_t beToHost64(uint64_t bigEndian) {
    return OSSwapBigToHostInt64(bigEndian);
  }

  uint64_t hostToBe64(uint64_t value) {
    return OSSwapHostToBigInt64(value);
  }
#else
#warning Not yet implemented properly for other platforms
  char *fakeMemory{nullptr};

  std::vector<int> getDolphinPids() {
    return {};
  }
  bool attachToProcess(int pid) {
    return false;
  }

  void detachFromProcess() {
  }

  void dolphin_memcpy(void *dest, std::size_t offset, std::size_t size) {
    if (!fakeMemory) {
      fakeMemory = new char[DOLPHIN_MEMORY_SIZE];
    }
  }

  int getAttachedPid() {
    return 0;
  }
#endif
}