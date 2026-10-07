#include "LStarCalcSettingsPopup.hpp"
#include "../Math/Calculator.hpp"
#include "../Data/State.hpp"
#include <sstream>
#include <iomanip>

using namespace geode::prelude;

bool LStarCalcSettingsPopup::setup() {
    this->setTitle("Frame Inspector (NaN GD L*)");

    auto winSize = m_mainLayer->getContentSize();

    auto inputs = State::get().getInputs();
    CalculationResult result = nandl::Calculator::solve(inputs, 0.5);

    if (result.valid) {
        State::get().setLStar(result.L_star);
        
        std::stringstream ss;
        ss << "Calculated L*: " << std::fixed << std::setprecision(4) << result.L_star;

        auto label = CCLabelBMFont::create(ss.str().c_str(), "bigFont.fnt");
        label->setScale(0.5f);
        label->setPosition(winSize / 2);
        m_mainLayer->addChild(label);
    } else {
        auto errorLabel = CCLabelBMFont::create("Calculation Failed or Empty Inputs", "goldFont.fnt");
        errorLabel->setScale(0.45f);
        errorLabel->setPosition(winSize / 2);
        m_mainLayer->addChild(errorLabel);
    }

    return true;
}

LStarCalcSettingsPopup* LStarCalcSettingsPopup::create() {
    auto ret = new LStarCalcSettingsPopup();
    if (ret && ret->initAnchored(320.f, 220.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}