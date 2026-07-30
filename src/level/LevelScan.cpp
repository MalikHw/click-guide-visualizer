#include "LevelScan.hpp"

#include <Geode/Bindings.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;
using namespace cocos2d;

namespace cgv {

namespace {

constexpr float kMinimumObstacleSize = 0.5f;

constexpr int kSlowSpeedId = 200;
constexpr int kNormalSpeedId = 201;
constexpr int kFastSpeedId = 202;
constexpr int kFasterSpeedId = 203;
constexpr int kFastestSpeedId = 1334;

constexpr float kSlowSpeed = 251.16f;
constexpr float kNormalSpeed = 311.58f;
constexpr float kFastSpeed = 387.42f;
constexpr float kFasterSpeed = 468.f;
constexpr float kFastestSpeed = 576.f;

bool isHazardType(GameObjectType type) {
    return type == GameObjectType::Hazard || type == GameObjectType::AnimatedHazard;
}

bool isSolidType(GameObjectType type) {
    return type == GameObjectType::Solid || type == GameObjectType::Slope;
}

bool portalMode(GameObjectType type, Gamemode& out) {
    switch (type) {
        case GameObjectType::CubePortal: out = Gamemode::Cube; return true;
        case GameObjectType::ShipPortal: out = Gamemode::Ship; return true;
        case GameObjectType::BallPortal: out = Gamemode::Ball; return true;
        case GameObjectType::UfoPortal: out = Gamemode::Ufo; return true;
        case GameObjectType::WavePortal: out = Gamemode::Wave; return true;
        case GameObjectType::RobotPortal: out = Gamemode::Robot; return true;
        case GameObjectType::SpiderPortal: out = Gamemode::Spider; return true;
        case GameObjectType::SwingPortal: out = Gamemode::Swing; return true;
        default: return false;
    }
}

bool speedPortal(int objectId, float& out) {
    switch (objectId) {
        case kSlowSpeedId: out = kSlowSpeed; return true;
        case kNormalSpeedId: out = kNormalSpeed; return true;
        case kFastSpeedId: out = kFastSpeed; return true;
        case kFasterSpeedId: out = kFasterSpeed; return true;
        case kFastestSpeedId: out = kFastestSpeed; return true;
        default: return false;
    }
}

bool shouldIgnore(GameObject* object) {
    if (!object) return true;
    if (object->m_isDontEnter) return true;
    if (object->m_hasNoEffects) return true;
    return false;
}

} // namespace

ScannedLevel scanLevelObstacles(GJBaseGameLayer* layer) {
    ScannedLevel scanned;
    if (!layer || !layer->m_objects) return scanned;

    CCArray* objects = layer->m_objects;
    unsigned int count = objects->count();
    scanned.obstacles.reserve(std::min<size_t>(count, kMaxScannedObstacles));

    for (unsigned int index = 0; index < count; ++index) {
        auto* object = typeinfo_cast<GameObject*>(objects->objectAtIndex(index));
        if (!object) continue;

        GameObjectType type = object->m_objectType;
        float objectX = object->getPositionX();

        Gamemode portalTarget = Gamemode::Cube;
        if (portalMode(type, portalTarget)) {
            scanned.modeChanges.push_back(ModeChange{objectX, portalTarget});
            continue;
        }

        float portalSpeed = 0.f;
        if (speedPortal(object->m_objectID, portalSpeed)) {
            scanned.speedChanges.push_back(SpeedChange{objectX, portalSpeed});
            continue;
        }

        if (shouldIgnore(object)) continue;
        if (scanned.obstacles.size() >= kMaxScannedObstacles) continue;

        Obstacle obstacle;
        if (isHazardType(type)) {
            obstacle.kind = ObstacleKind::Hazard;
        } else if (isSolidType(type)) {
            obstacle.kind = ObstacleKind::SolidWall;
        } else {
            continue;
        }

        CCRect rect = object->getObjectRect();
        if (rect.size.width < kMinimumObstacleSize || rect.size.height < kMinimumObstacleSize) continue;

        obstacle.leftX = rect.getMinX();
        obstacle.rightX = rect.getMaxX();
        obstacle.bottomY = rect.getMinY();
        obstacle.topY = rect.getMaxY();

        if (!std::isfinite(obstacle.leftX) || !std::isfinite(obstacle.bottomY)) continue;

        scanned.obstacles.push_back(obstacle);
    }

    std::stable_sort(scanned.obstacles.begin(), scanned.obstacles.end(),
                     [](Obstacle const& a, Obstacle const& b) { return a.leftX < b.leftX; });
    std::stable_sort(scanned.modeChanges.begin(), scanned.modeChanges.end(),
                     [](ModeChange const& a, ModeChange const& b) { return a.atX < b.atX; });
    std::stable_sort(scanned.speedChanges.begin(), scanned.speedChanges.end(),
                     [](SpeedChange const& a, SpeedChange const& b) { return a.atX < b.atX; });

    scanned.valid = true;
    return scanned;
}

Gamemode modeAt(ScannedLevel const& level, float x, Gamemode startMode) {
    Gamemode current = startMode;
    for (auto const& change : level.modeChanges) {
        if (change.atX > x) break;
        current = change.mode;
    }
    return current;
}

float speedAt(ScannedLevel const& level, float x, float startSpeed) {
    float current = startSpeed;
    for (auto const& change : level.speedChanges) {
        if (change.atX > x) break;
        current = change.unitsPerSecond;
    }
    return current;
}

} // namespace cgv
