#pragma once

#include <cstddef>

namespace cgv {

constexpr double kDefaultTickRate = 240.0;
constexpr double kMinPlausibleTickRate = 30.0;
constexpr double kMaxPlausibleTickRate = 2000.0;

bool isPlausibleTickRate(double rate);
double recoverTickRate(double declared, size_t frameSpan, float duration);

} // namespace cgv
