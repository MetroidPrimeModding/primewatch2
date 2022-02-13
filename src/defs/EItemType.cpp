#include "EItemType.hpp"

std::string itemTypeToName(EItemType type) {
  switch (type) {
    case EItemType::PowerBeam:
      return "Power Beam";
    case EItemType::IceBeam:
      return "Ice Beam";
    case EItemType::WaveBeam:
      return "Wave Beam";
    case EItemType::PlasmaBeam:
      return "Plasma Beam";
    case EItemType::Missiles:
      return "Missiles";
    case EItemType::ScanVisor:
      return "Scan Visor";
    case EItemType::MorphBallBombs:
      return "Morph Ball Bombs";
    case EItemType::PowerBombs:
      return "Power Bombs";
    case EItemType::Flamethrower:
      return "Flamethrower";
    case EItemType::ThermalVisor:
      return "Thermal Visor";
    case EItemType::ChargeBeam:
      return "Charge Beam";
    case EItemType::SuperMissile:
      return "Super Missile";
    case EItemType::GrappleBeam:
      return "Grapple Beam";
    case EItemType::XRayVisor:
      return "X-Ray Visor";
    case EItemType::IceSpreader:
      return "Ice Spreader";
    case EItemType::SpaceJumpBoots:
      return "Space Jump Boots";
    case EItemType::MorphBall:
      return "Morph Ball";
    case EItemType::CombatVisor:
      return "Combat Visor";
    case EItemType::BoostBall:
      return "Boost Ball";
    case EItemType::SpiderBall:
      return "Spider Ball";
    case EItemType::PowerSuit:
      return "Power Suit";
    case EItemType::GravitySuit:
      return "Gravity Suit";
    case EItemType::VariaSuit:
      return "Varia Suit";
    case EItemType::PhazonSuit:
      return "Phazon Suit";
    case EItemType::EnergyTanks:
      return "Energy Tanks";
    case EItemType::UnknownItem1:
      return "Unknown Item 1";
    case EItemType::HealthRefill:
      return "Health Refill";
    case EItemType::UnknownItem2:
      return "Unknown Item 2";
    case EItemType::Wavebuster:
      return "Wavebuster";
    case EItemType::Truth:
      return "Truth";
    case EItemType::Strength:
      return "Strength";
    case EItemType::Elder:
      return "Elder";
    case EItemType::Wild:
      return "Wild";
    case EItemType::Lifegiver:
      return "Lifegiver";
    case EItemType::Warrior:
      return "Warrior";
    case EItemType::Chozo:
      return "Chozo";
    case EItemType::Nature:
      return "Nature";
    case EItemType::Sun:
      return "Sun";
    case EItemType::World:
      return "World";
    case EItemType::Spirit:
      return "Spirit";
    case EItemType::Newborn:
      return "Newborn";
    default:
      return "Unknown";
  }
}
