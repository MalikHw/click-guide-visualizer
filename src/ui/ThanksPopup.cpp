#include "ThanksPopup.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <algorithm>

using namespace geode::prelude;
using namespace cocos2d;

namespace cgv {

namespace {

constexpr float kPopupWidth = 420.f;
constexpr float kPopupHeight = 290.f;
constexpr float kListWidth = 368.f;
constexpr float kListHeight = 196.f;
constexpr float kTitleScale = .9f;
constexpr float kTitleOffset = 22.f;
constexpr float kListCentreY = -12.f;
constexpr float kRowHeight = 34.f;
constexpr float kRowLabelInset = 12.f;
constexpr float kNameScale = .62f;
constexpr float kReasonScale = .42f;
constexpr float kNameOffsetY = 9.f;
constexpr float kReasonOffsetY = -8.f;
constexpr float kLabelWidthLimit = 344.f;
constexpr float kNameMinScale = .3f;
constexpr float kReasonMinScale = .24f;
constexpr unsigned char kReasonOpacity = 175;
constexpr ccColor4B kListBackground{0, 0, 0, 90};
constexpr ccColor4B kRowTint{255, 255, 255, 12};
constexpr ccColor3B kNameColor{255, 226, 130};
constexpr const char* kBoldFont = "bigFont.fnt";
constexpr const char* kBackground = "GJ_square01.png";

constexpr ThanksEntry kThanks[] = {
    {"LLMeowmerecat", "best friend, thanks so much for the help twin"},
    {"viperi", "awesome staff"},
    {"bodypasta", "helped me at my lowest"},
    {"flexa", "set up the server and helped from the beginning"},
    {"albeary", "second owner, awesome person"},
    {"tetarp / heggie", "nice person and staff"},
    {"marec", "helped out"},
    {"luck", "also helped me out at my lowest"},
    {"caw", "kind and awesome staff"},
    {"malikhw47", "nice person and checked my code for errors once"},
    {"all my lovely members", "thanks for using click guide visualizer"},
    {"maxnut", "GDReplayFormat, the gdr parser"},
    {"hyperbolus.net", "the online macro library"},
};

constexpr size_t kThanksCount = sizeof(kThanks) / sizeof(kThanks[0]);

} // namespace

ThanksPopup* ThanksPopup::create() {
    auto* popup = new ThanksPopup();
    if (popup && popup->init()) {
        popup->autorelease();
        return popup;
    }
    CC_SAFE_DELETE(popup);
    return nullptr;
}

bool ThanksPopup::init() {
    if (!Popup::init(kPopupWidth, kPopupHeight, kBackground)) return false;

    this->setTitle("Special Thanks", "goldFont.fnt", kTitleScale, kTitleOffset);
    this->buildScrollList();

    return true;
}

void ThanksPopup::buildScrollList() {
    m_scroll = ScrollLayer::create({kListWidth, kListHeight});
    if (!m_scroll) return;

    auto* background = CCScale9Sprite::create("square02_001.png");
    if (background) {
        background->setContentSize({kListWidth, kListHeight});
        background->setOpacity(kListBackground.a);
        background->setColor({kListBackground.r, kListBackground.g, kListBackground.b});
        m_mainLayer->addChildAtPosition(background, Anchor::Center, ccp(0.f, kListCentreY));
    }

    m_scroll->ignoreAnchorPointForPosition(false);
    m_scroll->setAnchorPoint(ccp(.5f, .5f));
    m_mainLayer->addChildAtPosition(m_scroll, Anchor::Center, ccp(0.f, kListCentreY));

    float contentHeight = std::max(kRowHeight * kThanksCount, kListHeight);
    m_scroll->m_contentLayer->setContentSize({kListWidth, contentHeight});

    for (size_t index = 0; index < kThanksCount; ++index) {
        this->addLine(kThanks[index], contentHeight - kRowHeight * index);
    }

    m_scroll->moveToTop();
}

void ThanksPopup::addLine(ThanksEntry const& entry, float rowTop) {
    float centreY = rowTop - kRowHeight * 0.5f;

    auto* name = CCLabelBMFont::create(entry.name, kBoldFont);
    if (name) {
        name->setAnchorPoint(ccp(0.f, .5f));
        name->limitLabelWidth(kLabelWidthLimit, kNameScale, kNameMinScale);
        name->setPosition(ccp(kRowLabelInset, centreY + kNameOffsetY));
        name->setColor(kNameColor);
        m_scroll->m_contentLayer->addChild(name);
    }

    auto* reason = CCLabelBMFont::create(entry.reason, kBoldFont);
    if (reason) {
        reason->setAnchorPoint(ccp(0.f, .5f));
        reason->limitLabelWidth(kLabelWidthLimit, kReasonScale, kReasonMinScale);
        reason->setPosition(ccp(kRowLabelInset, centreY + kReasonOffsetY));
        reason->setOpacity(kReasonOpacity);
        m_scroll->m_contentLayer->addChild(reason);
    }
}

} // namespace cgv
