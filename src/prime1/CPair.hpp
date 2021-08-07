#pragma once

#include "game_value.h"

template<class A, class B>
class CPair : public game_value<A::size + B::size> {
    static_assert(A::size > 0, "Attempting to create pair of an object with undefined size (A)");
    static_assert(B::size > 0, "Attempting to create pair of an object with undefined size (B)");
public:
    CPair(uint32_t base_ptr, uint32_t ptr_offset = 0) : game_value<A::size + B::size>(base_ptr, ptr_offset) {}

    A a = A(this->ptr(), 0x0);
    B b = B(this->ptr(), A::size);
};
