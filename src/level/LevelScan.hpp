#pragma once

#include "core/Gamemode.hpp"

#include <cstddef>
#include <vector>

class GJBaseGameLayer;

namespace cgv {

enum class ObstacleKind {
    Hazard,
    SolidWall,
};

struct Obstacle {
    float leftX = 0.f;
    float rightX = 0.f;
    float topY = 0.f;
    float bottomY = 0.f;
    ObstacleKind kind = ObstacleKind::Hazard;
};

struct ModeChange {
    float atX = 0.f;
    Gamemode mode = Gamemode::Cube;
};

struct SpeedChange {
    float atX = 0.f;
    float unitsPerSecond = 311.58f;
};

struct ScannedLevel {
    std::vector<Obstacle> obstacles;
    std::vector<ModeChange> modeChanges;
    std::vector<SpeedChange> speedChanges;
    bool valid = false;

    void clear() {
        obstacles.clear();
        modeChanges.clear();
        speedChanges.clear();
        valid = false;
    }
};

constexpr size_t kMaxScannedObstacles = 60000;

ScannedLevel scanLevelObstacles(GJBaseGameLayer* layer);

Gamemode modeAt(ScannedLevel const& level, float x, Gamemode startMode);
float speedAt(ScannedLevel const& level, float x, float startSpeed);

} // namespace cgv
