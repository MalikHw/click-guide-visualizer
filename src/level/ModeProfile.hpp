#pragma once

#include "core/Gamemode.hpp"

namespace cgv {

enum class InputStyle {
    Tap,
    Hold,
    Repeat,
};

struct ModeProfile {
    float gravity;
    float jumpImpulse;
    float holdAcceleration;
    float terminalSpeed;
    float hitboxHalfWidth;
    float hitboxHalfHeight;
    float lookaheadUnits;
    float minimumSpacingUnits;
    InputStyle style;
    bool needsGround;
    bool ceilingMatters;
};

ModeProfile profileFor(Gamemode mode);
bool modeIsFlight(Gamemode mode);

} // namespace cgv
