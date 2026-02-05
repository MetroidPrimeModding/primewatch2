#pragma once

#include "GameDefinitions.hpp"
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
namespace GameObjectRenderers {
  extern bool render_exact_values;
  void render(const GameDefinitions::GameMember &member, bool addTree = true, bool derefPointer = false);

  void renderEnumOrStruct(const GameDefinitions::GameMember &member, bool addTree);
  void primitiveRenderer(const GameDefinitions::GameMember &member);
  void CVector3fRenderer(const GameDefinitions::GameMember &member);
  void CQuaternionRenderer(const GameDefinitions::GameMember &member);
  void CTransformRenderer(const GameDefinitions::GameMember &member);
  void CMatrix4fRenderer(const GameDefinitions::GameMember &member);
  void renderVector(const GameDefinitions::GameMember &member);
  void renderArray(const GameDefinitions::GameMember &member);
  void SObjectTagRenderer(const GameDefinitions::GameMember &member);

  using RenderFunc = void (*)(const GameDefinitions::GameMember &);

  extern std::unordered_map<std::string, RenderFunc> specialRenderers;
} // namespace GameObjectRenderers