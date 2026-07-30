#include "Gamemode.hpp"

#include <Geode/Bindings.hpp>

namespace cgv {

Gamemode gamemodeOf(PlayerObject* player) {
    if (!player) return Gamemode::Cube;
    if (player->m_isSwing) return Gamemode::Swing;
    if (player->m_isDart) return Gamemode::Wave;
    if (player->m_isSpider) return Gamemode::Spider;
    if (player->m_isRobot) return Gamemode::Robot;
    if (player->m_isBird) return Gamemode::Ufo;
    if (player->m_isBall) return Gamemode::Ball;
    if (player->m_isShip) return Gamemode::Ship;
    return Gamemode::Cube;
}

const char* gamemodeName(Gamemode mode) {
    switch (mode) {
        case Gamemode::Cube: return "Cube";
        case Gamemode::Ship: return "Ship";
        case Gamemode::Ball: return "Ball";
        case Gamemode::Ufo: return "UFO";
        case Gamemode::Wave: return "Wave";
        case Gamemode::Robot: return "Robot";
        case Gamemode::Spider: return "Spider";
        case Gamemode::Swing: return "Swing";
    }
    return "Cube";
}

const char* gamemodeSettingKey(Gamemode mode) {
    switch (mode) {
        case Gamemode::Cube: return "mode-cube";
        case Gamemode::Ship: return "mode-ship";
        case Gamemode::Ball: return "mode-ball";
        case Gamemode::Ufo: return "mode-ufo";
        case Gamemode::Wave: return "mode-wave";
        case Gamemode::Robot: return "mode-robot";
        case Gamemode::Spider: return "mode-spider";
        case Gamemode::Swing: return "mode-swing";
    }
    return "mode-cube";
}

} // namespace cgv
