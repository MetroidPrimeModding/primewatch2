#pragma once

#include <string>

enum class EItemType {
  Invalid = -1,
  PowerBeam = 0,
  IceBeam = 1,
  WaveBeam = 2,
  PlasmaBeam = 3,
  Missiles = 4,
  ScanVisor = 5,
  MorphBallBombs = 6,
  PowerBombs = 7,
  Flamethrower = 8,
  ThermalVisor = 9,
  ChargeBeam = 10,
  SuperMissile = 11,
  GrappleBeam = 12,
  XRayVisor = 13,
  IceSpreader = 14,
  SpaceJumpBoots = 15,
  MorphBall = 16,
  CombatVisor = 17,
  BoostBall = 18,
  SpiderBall = 19,
  PowerSuit = 20,
  GravitySuit = 21,
  VariaSuit = 22,
  PhazonSuit = 23,
  EnergyTanks = 24,
  UnknownItem1 = 25,
  HealthRefill = 26,
  UnknownItem2 = 27,
  Wavebuster = 28,
  Truth = 29,
  Strength = 30,
  Elder = 31,
  Wild = 32,
  Lifegiver = 33,
  Warrior = 34,
  Chozo = 35,
  Nature = 36,
  Sun = 37,
  World = 38,
  Spirit = 39,
  Newborn = 40,

  /* This must remain at the end of the list */
  Max
};

std::string itemTypeToName(EItemType type);