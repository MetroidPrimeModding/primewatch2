#pragma once

#include "game_value.h"

namespace rstl {
  template<typename T>
  class string : public game_value<16> {
  public:
    explicit string(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value<16>(base_ptr, ptr_offset) {}

    game_ptr<T> data = game_ptr<T>(this->ptr(), 0x0);
    game_u32 unk2 = game_u32(this->ptr(), 0x4);
    game_u32 unk3 = game_u32(this->ptr(), 0x8);
    game_u32 unk4 = game_u32(this->ptr(), 0xC);

    inline std::string read_str() {
      uint32_t maxLen = 512;
      std::string res;
      uint32_t pos = data.read();
      for (uint32_t i = 0; i < maxLen; i++) {
        char c = game_u8(pos, i).read();
        if (c == 0) {
          break;
        }
        res += c;
      }
      return res;
    }
  };


  template<class T>
  class vector : public game_value<12> {
    static_assert(T::size > 0, "Attempting to create vector of an object with undefined size");
  public:
    vector(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_u32 end = game_u32(ptr(), 0x0);
    game_u32 size = game_u32(ptr(), 0x4);
    game_ptr<T> first = game_ptr<T>(ptr(), 0x8);

    inline T operator[](std::size_t idx) const {
      uint32_t count = size.read();
      assert(idx < count); // Error out in debug mode
      return T(first.read(), (idx % count) * T::size); //In production, take the modulus and hope for the best
    }

  };

  template<class T>
  class auto_ptr : public game_value<8> {
  public:
    auto_ptr(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value(base_ptr, ptr_offset) {}

    game_u32 unknown = game_u32(ptr(), 0x0);
    game_u8 referenced = game_u8(ptr(), 0x0);
    game_ptr<T> dataPtr = game_ptr<T>(ptr(), 0x4);
  };
}
