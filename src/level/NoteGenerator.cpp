#include "NoteGenerator.hpp"

#include "level/ModeProfile.hpp"

#include <algorithm>
#include <cmath>

namespace cgv {

namespace {

constexpr float kGroundProbeDepth = 4.f;
constexpr float kLandingSlack = 2.f;
constexpr float kCorridorProbeStep = 6.f;
constexpr float kCorridorSearchRange = 240.f;
constexpr float kFlightClimbMargin = 0.35f;
constexpr float kMinimumSpeed = 1.f;
constexpr float kFallLimitUnits = 2000.f;

size_t firstNear(ScannedLevel const& level, float x, float back) {
    auto found = std::lower_bound(level.obstacles.begin(), level.obstacles.end(), x - back,
                                  [](Obstacle const& o, float v) { return o.rightX < v; });
    return static_cast<size_t>(found - level.obstacles.begin());
}

bool boxHits(Obstacle const& o, float left, float right, float bottom, float top) {
    if (o.rightX < left || o.leftX > right) return false;
    if (o.topY < bottom || o.bottomY > top) return false;
    return true;
}

bool blockedAt(ScannedLevel const& level, float x, float y, ModeProfile const& profile) {
    float left = x - profile.hitboxHalfWidth;
    float right = x + profile.hitboxHalfWidth;
    float bottom = y - profile.hitboxHalfHeight;
    float top = y + profile.hitboxHalfHeight;

    for (size_t i = firstNear(level, x, profile.hitboxHalfWidth * 2.f); i < level.obstacles.size(); ++i) {
        auto const& o = level.obstacles[i];
        if (o.leftX > right) break;
        if (boxHits(o, left, right, bottom, top)) return true;
    }
    return false;
}

bool hazardAhead(ScannedLevel const& level, float x, float y, ModeProfile const& profile) {
    float left = x + profile.hitboxHalfWidth;
    float right = x + profile.lookaheadUnits;
    float bottom = y - profile.hitboxHalfHeight;
    float top = y + profile.hitboxHalfHeight;

    for (size_t i = firstNear(level, x, profile.lookaheadUnits); i < level.obstacles.size(); ++i) {
        auto const& o = level.obstacles[i];
        if (o.leftX > right) break;
        if (o.kind != ObstacleKind::Hazard) continue;
        if (boxHits(o, left, right, bottom, top)) return true;
    }
    return false;
}

bool wallAhead(ScannedLevel const& level, float x, float y, ModeProfile const& profile) {
    float left = x + profile.hitboxHalfWidth;
    float right = x + profile.hitboxHalfWidth + profile.lookaheadUnits * 0.45f;
    float bottom = y - profile.hitboxHalfHeight * 0.6f;
    float top = y + profile.hitboxHalfHeight;

    for (size_t i = firstNear(level, x, profile.lookaheadUnits); i < level.obstacles.size(); ++i) {
        auto const& o = level.obstacles[i];
        if (o.leftX > right) break;
        if (o.kind != ObstacleKind::SolidWall) continue;
        if (!boxHits(o, left, right, bottom, top)) continue;
        if (o.topY > y + profile.hitboxHalfHeight * 0.5f) return true;
    }
    return false;
}

bool groundBelow(ScannedLevel const& level, float x, float y, ModeProfile const& profile, float& surfaceOut) {
    float left = x - profile.hitboxHalfWidth * 0.8f;
    float right = x + profile.hitboxHalfWidth * 0.8f;
    float probeBottom = y - profile.hitboxHalfHeight - kGroundProbeDepth;
    float probeTop = y - profile.hitboxHalfHeight + kLandingSlack;

    bool found = false;
    float best = 0.f;

    for (size_t i = firstNear(level, x, profile.hitboxHalfWidth * 2.f); i < level.obstacles.size(); ++i) {
        auto const& o = level.obstacles[i];
        if (o.leftX > right) break;
        if (o.kind != ObstacleKind::SolidWall) continue;
        if (o.rightX < left) continue;
        if (o.topY < probeBottom || o.topY > probeTop) continue;
        if (!found || o.topY > best) {
            best = o.topY;
            found = true;
        }
    }

    if (found) surfaceOut = best;
    return found;
}

bool gapAhead(ScannedLevel const& level, float x, float y, ModeProfile const& profile, float groundY) {
    float probeX = x + profile.lookaheadUnits * 0.75f;
    float surface = 0.f;
    if (groundBelow(level, probeX, y, profile, surface)) return false;

    float top = y - profile.hitboxHalfHeight;
    float left = probeX - profile.hitboxHalfWidth;
    float right = probeX + profile.hitboxHalfWidth;

    for (size_t i = firstNear(level, probeX, profile.lookaheadUnits); i < level.obstacles.size(); ++i) {
        auto const& o = level.obstacles[i];
        if (o.leftX > right) break;
        if (o.kind != ObstacleKind::SolidWall) continue;
        if (o.rightX < left) continue;
        if (o.topY <= top && o.topY > groundY - profile.lookaheadUnits) return false;
    }
    return true;
}

bool findCorridorCentre(ScannedLevel const& level, float x, float fromY,
                        ModeProfile const& profile, float& centreOut) {
    float lowest = fromY;
    float highest = fromY;

    while (lowest - kCorridorProbeStep > fromY - kCorridorSearchRange) {
        if (blockedAt(level, x, lowest - kCorridorProbeStep, profile)) break;
        lowest -= kCorridorProbeStep;
    }
    while (highest + kCorridorProbeStep < fromY + kCorridorSearchRange) {
        if (blockedAt(level, x, highest + kCorridorProbeStep, profile)) break;
        highest += kCorridorProbeStep;
    }

    if (highest - lowest < kCorridorProbeStep) return false;

    centreOut = (lowest + highest) * 0.5f;
    return true;
}

void appendGroundNotes(ScannedLevel const& level, GenerationInput const& input,
                       std::vector<GeneratedNote>& notes, float& x, float& y,
                       Gamemode mode, float horizonX) {
    ModeProfile profile = profileFor(mode);
    float verticalSpeed = 0.f;
    bool grounded = true;
    float lastNoteX = -1e9f;

    while (x < horizonX && notes.size() < kMaxGeneratedNotes) {
        if (modeAt(level, x, input.startMode) != mode) return;

        float speed = std::max(speedAt(level, x, input.startSpeed), kMinimumSpeed);
        float surface = y - profile.hitboxHalfHeight;
        bool onSurface = groundBelow(level, x, y, profile, surface);

        if (onSurface && verticalSpeed <= 0.f) {
            y = surface + profile.hitboxHalfHeight;
            verticalSpeed = 0.f;
            grounded = true;
        } else {
            grounded = false;
        }

        bool canAct = profile.needsGround ? grounded : true;

        if (canAct && (x - lastNoteX) > profile.minimumSpacingUnits) {
            bool wants = hazardAhead(level, x, y, profile)
                      || wallAhead(level, x, y, profile)
                      || gapAhead(level, x, y, profile, surface);

            if (wants) {
                GeneratedNote note;
                note.levelX = x;
                note.levelY = y;
                note.mode = mode;
                note.holdSeconds = profile.style == InputStyle::Hold ? 0.08f : 0.f;
                notes.push_back(note);

                verticalSpeed = profile.jumpImpulse > 0.f ? profile.jumpImpulse : profile.terminalSpeed * 0.5f;
                grounded = false;
                lastNoteX = x;
            }
        }

        if (!grounded) {
            verticalSpeed -= profile.gravity * kGenerationStep;
            if (profile.terminalSpeed > 0.f) {
                verticalSpeed = std::clamp(verticalSpeed, -profile.terminalSpeed, profile.terminalSpeed);
            }
            y += verticalSpeed * kGenerationStep;
        }

        x += speed * kGenerationStep;
        if (!std::isfinite(x) || !std::isfinite(y)) return;
        if (y < input.startY - kFallLimitUnits) return;
    }
}

void appendFlightNotes(ScannedLevel const& level, GenerationInput const& input,
                       std::vector<GeneratedNote>& notes, float& x, float& y,
                       Gamemode mode, float horizonX) {
    ModeProfile profile = profileFor(mode);
    float lastNoteX = -1e9f;

    while (x < horizonX && notes.size() < kMaxGeneratedNotes) {
        if (modeAt(level, x, input.startMode) != mode) return;

        float speed = std::max(speedAt(level, x, input.startSpeed), kMinimumSpeed);
        float probeX = x + profile.lookaheadUnits;

        float centre = y;
        if (findCorridorCentre(level, probeX, y, profile, centre)) {
            bool mustClimb = centre > y + profile.hitboxHalfHeight * kFlightClimbMargin;

            if (mustClimb && (x - lastNoteX) > profile.minimumSpacingUnits) {
                float rise = centre - y;
                float travelSeconds = std::max(rise, 1.f) / std::max(profile.holdAcceleration > 0.f
                                                                     ? profile.holdAcceleration * 0.25f
                                                                     : profile.terminalSpeed, 1.f);

                GeneratedNote note;
                note.levelX = x;
                note.levelY = y;
                note.mode = mode;
                note.holdSeconds = std::clamp(travelSeconds, 0.05f, 0.6f);
                notes.push_back(note);
                lastNoteX = x;
            }

            y += (centre - y) * 0.25f;
        }

        x += std::max(speed * kGenerationStep, kFlightSampleUnits * kGenerationStep);
        if (!std::isfinite(x) || !std::isfinite(y)) return;
    }
}

} // namespace

std::vector<GeneratedNote> generateNotes(ScannedLevel const& level, GenerationInput const& input) {
    std::vector<GeneratedNote> notes;
    if (!level.valid || level.obstacles.empty()) return notes;

    float x = input.startX;
    float y = input.startY;

    while (x < input.horizonX && notes.size() < kMaxGeneratedNotes) {
        Gamemode mode = modeAt(level, x, input.startMode);
        float before = x;

        if (modeIsFlight(mode)) {
            appendFlightNotes(level, input, notes, x, y, mode, input.horizonX);
        } else {
            appendGroundNotes(level, input, notes, x, y, mode, input.horizonX);
        }

        if (x <= before) x = before + kFlightSampleUnits;
    }

    std::stable_sort(notes.begin(), notes.end(),
                     [](GeneratedNote const& a, GeneratedNote const& b) { return a.levelX < b.levelX; });

    return notes;
}

} // namespace cgv
