#include "GameObject.hpp"

#include <utility>
#include "imgui.h"

GameObject::GameObject(std::string name, GameDefinitions::GameStruct gameStruct, uint32_t address)
    : name(std::move(name)), gameStruct(std::move(gameStruct)), address(address) {

}

void GameObject::renderGui(bool addTree) {
  if (addTree) {
    if (!ImGui::TreeNode((gameStruct.name + " " + name).c_str())) {
      return;
    }
  }
  for (auto &[_, child]: gameStruct.members_by_name) {
    if (GameDefinitions::structs_by_name.count(child.type)) {
      auto s = GameDefinitions::structs_by_name[child.type];
      GameObject obj(child.name, s, address + child.offset);
      obj.renderGui();
    }
  }
  if (addTree) {
    ImGui::TreePop();
  }
}
