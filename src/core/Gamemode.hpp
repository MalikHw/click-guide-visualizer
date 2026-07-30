#pragma once

#include <cstddef>

namespace cocos2d {
class CCPoint;
}

class PlayerObject;

namespace cgv {

enum class Gamemode {
    Cube,
    Ship,
    Ball,
    Ufo,
    Wave,
    Robot,
    Spider,
    Swing,
};

constexpr size_t kGamemodeCount = 8;

Gamemode gamemodeOf(PlayerObject* player);
const char* gamemodeName(Gamemode mode);
const char* gamemodeSettingKey(Gamemode mode);

} // namespace cgv
