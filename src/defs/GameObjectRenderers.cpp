#include "GameObjectRenderers.hpp"

#include <imgui.h>
#include <fmt/format.h>
#include "GameMemory.h"

using namespace std;
using namespace GameDefinitions;

namespace GameObjectRenderers {
  unordered_map<string, RenderFunc> specialRenderers{ // NOLINT(cert-err58-cpp) // it won't throw (or at least I don't care if it does right now)
      {"u8",          &primitiveRenderer},
      {"u16",         &primitiveRenderer},
      {"u32",         &primitiveRenderer},
      {"u64",         &primitiveRenderer},
      {"i8",          &primitiveRenderer},
      {"i16",         &primitiveRenderer},
      {"i32",         &primitiveRenderer},
      {"i64",         &primitiveRenderer},
      {"f32",         &primitiveRenderer},
      {"f64",         &primitiveRenderer},
      {"bool",        &primitiveRenderer},
      {"CVector3f",   &CVector3fRenderer},
      {"CQuaternion", &CQuaternionRenderer},
  };

  void render(const GameMember &member, bool addTree) {
    auto &typeName = member.type;
    if (specialRenderers.count(typeName)) {
      specialRenderers[typeName](member);
    } else {
      if (typeName.rfind("rstl::vector", 0) != string::npos) {
        renderVector(member);
      } else {
        renderEnumOrStruct(member, addTree);
      }
    }
  }

  void hoverTooltip(const GameMember &member) {
    if (ImGui::IsItemHovered()) {
      string msg;
      if (member.pointer) {
        msg += "*";
      }
      msg += member.type;
      if (member.arrayLength.has_value()) {
        msg += fmt::format("[{}]", *member.arrayLength);
      }
      msg += fmt::format(" {:#08x}", member.offset);
      if (member.bit.has_value() || member.bitLength.has_value()) {
        uint32_t bit = member.bit.value_or(0);
        uint32_t length = member.bitLength.value_or(0);
        msg += fmt::format("[bit {}; len {}]", bit, length);
      }
      ImGui::SetTooltip("%s", msg.c_str());
    }
  }

  void renderEnum(const GameMember &member) {
    auto gameEnum = GameDefinitions::enumByName(member.type);
    if (!gameEnum.has_value()) {
      ImGui::Text("Unknown enum %s", member.type.c_str());
      return;
    }
    uint32_t value = GameMemory::read_u32(member.offset);
    string name = "unknown";
    if (gameEnum->values_by_value.count(value)) {
      name = gameEnum->values_by_value[value].name;
    }
    string msg = fmt::format("{0} {1} ({2:d}/{2:#x}/{2:#b})", member.name, name, value);
    ImGui::Text("%s", msg.c_str());
  }

  void renderEnumOrStruct(const GameMember &member, bool addTree) {
    if (GameDefinitions::enumByName(member.type).has_value()) {
      renderEnum(member);
      return;
    }

    auto gameStruct = GameDefinitions::structByName(member.type);
    if (!gameStruct.has_value()) {
      ImGui::Text("Unknown type %s", member.type.c_str());
      hoverTooltip(member);
      return;
    }

    if (addTree) {
      string label = fmt::format("{0}###{0}{1:08x}", member.name, member.offset);
      bool open = ImGui::CollapsingHeader(label.c_str());
      hoverTooltip(member);
      if (!open) return;
      ImGui::Indent();
    }

    if (member.offset == 0) {
      ImGui::Text("null");
    } else {
      for (auto &extends: gameStruct->extends) {
        render(GameMember{
            .name = extends,
            .type = extends,
            .offset = member.offset,
        });
      }

      for (auto &child: gameStruct->members_by_order) {
        uint32_t addr = member.offset + child.offset;
        string name = child.name;

        if (child.pointer) {
          addr = GameMemory::read_u32(addr);
        }

        GameMember childCopy = child;
        childCopy.offset = addr;
        render(childCopy);
      }
    }

    if (addTree) {
      ImGui::Unindent();
    }
  }

  uint64_t getBits(uint64_t v, uint32_t bit, uint32_t length) {
    uint64_t mask = (1ULL << length) - 1;
    if (length == 64) {
      mask = ~0;
    }
    return (v >> bit) & mask;
  }

  void primitiveRenderer(const GameMember &member) {
    string name = member.name;
    if (member.pointer) {
      name = "*" + name;
    }

    string itemText = name + " ";
    string clip;
    auto &typ = member.type;
    if (typ == "u8" && member.pointer) {
      // c string
      string val = member.read_string();
      itemText += fmt::format("\"{}\"", val);
      clip = val;
    } else if (typ == "bool") {
      bool v = member.read_bool();
      if (v) {
        itemText += "true";
        clip = "true";
      } else {
        itemText += "false";
        clip = "false";
      }
    } else if (typ[0] == 'i') {
      uint64_t v;
      if (typ == "i8") {
        v = member.read_u8();
      } else if (typ == "i16") {
        v = member.read_u16();
      } else if (typ == "i32") {
        v = member.read_u32();
      } else {
        v = member.read_u64();
      }
      itemText += fmt::format("{0:d}/{0:#x}", static_cast<int64_t>(v));
      clip = fmt::format("{:d}", v);
    } else if (typ[0] == 'u') {
      uint64_t v;
      if (typ == "u8") {
        v = member.read_u8();
      } else if (typ == "u16") {
        v = member.read_u16();
      } else if (typ == "u32") {
        v = member.read_u32();
      } else {
        v = member.read_u64();
      }
      itemText += fmt::format("{0:d}/{0:#x}", v);
      clip = fmt::format("{:d}", v);
    } else if (typ[0] == 'f') {
      if (typ == "f32") {
        float f = member.read_f32();
        itemText += fmt::format("{:0.3f}", f);
        clip = fmt::format("{:f}", f);
      } else {
        double d = member.read_f64();
        itemText += fmt::format("{:0.3f}", d);
        clip = fmt::format("{:f}", d);
      }
    } else {
      itemText += fmt::format("Unknown number type {}", typ);
    }
    ImGui::Text("%s", itemText.c_str());
    if (ImGui::IsItemClicked()) {
      ImGui::SetClipboardText(clip.c_str());
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    hoverTooltip(member);
  }

  void CVector3fRenderer(const GameMember &member) {
    uint32_t addr = member.offset;
    float x = GameMemory::read_float(addr);
    float y = GameMemory::read_float(addr + 4);
    float z = GameMemory::read_float(addr + 8);

    string msg = fmt::format("{} [{:0.3f}, {:0.3f}, {:0.3f}]", member.name, x, y, z);
    ImGui::Text("%s", msg.c_str());
    if (ImGui::IsItemClicked()) {
      string clip = fmt::format("{:f}, {:f}, {:f}", x, y, z);
      ImGui::SetClipboardText(clip.c_str());
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    hoverTooltip(member);
  }

  void CQuaternionRenderer(const GameMember &member) {
    uint32_t addr = member.offset;
    float x = GameMemory::read_float(addr);
    float y = GameMemory::read_float(addr + 4);
    float z = GameMemory::read_float(addr + 8);
    float w = GameMemory::read_float(addr + 12);

    string msg = fmt::format("{} [{:0.3f}, {:0.3f}, {:0.3f}, {:0.3f}]", member.name, x, y, z, w);

    ImGui::Text("%s", msg.c_str());
    if (ImGui::IsItemClicked()) {
      string clip = fmt::format("{:f}, {:f}, {:f}, {:f}", x, y, z, w);
      ImGui::SetClipboardText(clip.c_str());
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    hoverTooltip(member);
  }

  void renderVector(const GameMember &member) {
    string label = fmt::format("{}###{:08x}", member.name, member.offset);
    bool open = ImGui::CollapsingHeader(label.c_str());
    hoverTooltip(member);
    if (!open) return;
    ImGui::Indent();

    ImGui::PushID(label.c_str());

    uint32_t end = member.memberByName("end")->read_u32();
    uint32_t size = member.memberByName("size")->read_u32();
    ImGui::Text("size: %d max size: %d", end, size);

    ImGuiStorage *storage = ImGui::GetStateStorage();
    int index = storage->GetInt(ImGui::GetID("index"), 0);

    ImGui::InputInt("Index", &index);

    if (index >= end) index = end - 1;
    if (index < 0) index = 0;

    storage->SetInt(ImGui::GetID("index"), index);

    auto first = *member.memberByName("first");
    auto baseOffset = first.offset;
    auto sizePer = GameDefinitions::structByName(first.type)->size;

    ImGui::Indent();
    GameMember vecItem = first;
    vecItem.offset += sizePer * index;
    vecItem.name = fmt::format("{:d}", index);
    ImGui::Text("%s", vecItem.name.c_str());
    render(vecItem, false);
    ImGui::Unindent();

    ImGui::PopID();
    ImGui::Unindent();
  }
}

