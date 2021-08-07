#pragma once

#include "CEntity.hpp"
#include "prime1/CTransform.hpp"

class CActor : public CEntity {
public:
  CActor(uint32_t base_ptr, uint32_t ptr_offset = 0) : CEntity(base_ptr, ptr_offset) {}

  CTransform transform = CTransform(ptr(), 0x34);

  inline void doGui() {
    if (ImGui::TreeNode("CActor")) {
      ImGui::Text(
          "Pos: %0.02f, %0.02f, %0.02f",
          transform.matrix[3].read(),
          transform.matrix[7].read(),
          transform.matrix[11].read()
          );

      ImGui::TreePop();
    }
  }
};
