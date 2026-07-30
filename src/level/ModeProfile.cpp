#include "ModeProfile.hpp"

namespace cgv {

namespace {

constexpr ModeProfile kCube{2900.f, 620.f, 0.f, 1000.f, 15.f, 15.f, 90.f, 18.f, InputStyle::Tap, true, false};
constexpr ModeProfile kShip{2400.f, 0.f, 4200.f, 900.f, 15.f, 10.f, 150.f, 6.f, InputStyle::Hold, false, true};
constexpr ModeProfile kBall{2900.f, 0.f, 0.f, 1000.f, 15.f, 15.f, 80.f, 20.f, InputStyle::Tap, true, true};
constexpr ModeProfile kUfo{2800.f, 480.f, 0.f, 900.f, 15.f, 12.f, 110.f, 14.f, InputStyle::Repeat, false, true};
constexpr ModeProfile kWave{0.f, 0.f, 0.f, 0.f, 10.f, 10.f, 130.f, 5.f, InputStyle::Hold, false, true};
constexpr ModeProfile kRobot{2900.f, 600.f, 1200.f, 1000.f, 15.f, 15.f, 95.f, 18.f, InputStyle::Hold, true, false};
constexpr ModeProfile kSpider{2900.f, 0.f, 0.f, 1000.f, 15.f, 15.f, 85.f, 20.f, InputStyle::Tap, true, true};
constexpr ModeProfile kSwing{2400.f, 0.f, 0.f, 900.f, 12.f, 12.f, 120.f, 10.f, InputStyle::Hold, false, true};

} // namespace

ModeProfile profileFor(Gamemode mode) {
    switch (mode) {
        case Gamemode::Cube: return kCube;
        case Gamemode::Ship: return kShip;
        case Gamemode::Ball: return kBall;
        case Gamemode::Ufo: return kUfo;
        case Gamemode::Wave: return kWave;
        case Gamemode::Robot: return kRobot;
        case Gamemode::Spider: return kSpider;
        case Gamemode::Swing: return kSwing;
    }
    return kCube;
}

bool modeIsFlight(Gamemode mode) {
    switch (mode) {
        case Gamemode::Ship:
        case Gamemode::Ufo:
        case Gamemode::Wave:
        case Gamemode::Swing:
            return true;
        default:
            return false;
    }
}

} // namespace cgv
