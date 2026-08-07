#pragma once

#include <cocos2d.h>
#include <string>

namespace cgv {

constexpr int kToastZOrder = 2000;
constexpr float kToastFadeSeconds = 0.35f;
constexpr float kToastMargin = 14.f;
constexpr float kToastScale = 0.5f;
constexpr float kToastDrift = 10.f;

void showToast(std::string const& text);
void showToast(std::string const& text, cocos2d::ccColor3B colour);

} // namespace cgv
