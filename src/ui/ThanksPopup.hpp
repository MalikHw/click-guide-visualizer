#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>

namespace cgv {

struct ThanksEntry {
    const char* name;
    const char* reason;
};

class ThanksPopup : public geode::Popup {
public:
    static ThanksPopup* create();

protected:
    bool init();

private:
    void buildScrollList();
    void addLine(ThanksEntry const& entry, float rowTop);

    geode::ScrollLayer* m_scroll = nullptr;
};

} // namespace cgv
