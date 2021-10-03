#pragma once

#include <cstdint>
#include "./GameDefinitions.hpp"

class GameObject {
public:
  GameObject(std::string name, GameDefinitions::GameStruct gameStruct, uint32_t address);

  void renderGui(bool addTree = true);

  std::string name;
  GameDefinitions::GameStruct gameStruct;
  uint32_t address;
};


