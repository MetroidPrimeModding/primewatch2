#include <fmt/format.h>
#include "AreaUtils.hpp"
#include "GameMemory.h"

using namespace GameDefinitions;
using namespace std;

namespace AreaUtils {
  vector<GameMember> getAreas() {
    GameMember stateManager = g_stateManager;
    auto world = stateManager.memberByName("world");
    if (!world) return {};
    auto areas = world->memberByName("areas");
    if (!areas) return {};

    // loop thru the vector
    uint32_t end = (*areas)["end"].read_u32();
//    uint32_t size = areas->memberByName("size")->read_u32();

    vector<GameMember> result;

    auto first = (*areas)["first"];
    auto sizePer = GameDefinitions::structByName(first.type)->size;
    for (int i = 0; i < end; i++) {
      GameMember vecItem = first;
      vecItem.offset += sizePer * i;
      GameMember area = vecItem["value"];
      area.name = fmt::format("area {}", i);
      result.push_back(area);
    }
    return result;
  }
}