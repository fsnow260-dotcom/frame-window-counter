#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

class LStarCalcSettingsPopup : public geode::Popup<> {
protected:
    bool setup() override;

public:
    static LStarCalcSettingsPopup* create();
};