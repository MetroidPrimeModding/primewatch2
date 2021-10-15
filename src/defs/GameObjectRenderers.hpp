#pragma once


#include <unordered_map>
#include <functional>
#include <optional>
#include <string>
#include "GameDefinitions.hpp"

namespace GameObjectRenderers {
  void render(const GameDefinitions::GameMember &member, bool addTree = true);

  void renderEnumOrStruct(const GameDefinitions::GameMember &member, bool addTree);
  void primitiveRenderer(const GameDefinitions::GameMember &member);
  void CVector3fRenderer(const GameDefinitions::GameMember &member);
  void CQuaternionRenderer(const GameDefinitions::GameMember &member);
  void CTransformRenderer(const GameDefinitions::GameMember &member);
  void CMatrix4fRenderer(const GameDefinitions::GameMember &member);
  void renderVector(const GameDefinitions::GameMember &member);

  using RenderFunc = void (*)(const GameDefinitions::GameMember&);

  extern std::unordered_map<std::string, RenderFunc> specialRenderers;
}