#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../Audio/SoundManager.hpp"
#include "../Data/State.hpp"
#include "../Math/Calculator.hpp"

using namespace geode::prelude;

class $modify(MyPlayLayerHook, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_hudLabel = nullptr;
        int m_currentFrame = 0;
        int m_lastProcessedFrame = -1;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        m_fields->m_currentFrame = 0;
        m_fields->m_lastProcessedFrame = -1;

        SoundManager::get().init();

        auto label = CCLabelBMFont::create("Frame Counter: Ready", "bigFont.fnt");
        label->setScale(0.45f);
        label->setAnchorPoint({0.0f, 1.0f});
        label->setPosition({12.0f, CCDirector::sharedDirector()->getWinSize().height - 12.0f});
        label->setZOrder(1000);
        label->setID("frame-window-hud-label"_spr);

        if (this->m_uiLayer) {
            this->m_uiLayer->addChild(label);
        } else {
            this->addChild(label);
        }
        m_fields->m_hudLabel = label;

        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!m_fields->m_hudLabel) return;

        if (!State::get().isLabelVisible()) {
            m_fields->m_hudLabel->setVisible(false);
            return;
        }

        m_fields->m_hudLabel->setVisible(true);

        m_fields->m_currentFrame++;
        int currentFrame = m_fields->m_currentFrame;

        std::string hudText = State::get().getFormattedHUDString(currentFrame);
        m_fields->m_hudLabel->setString(hudText.c_str());

        if (currentFrame != m_fields->m_lastProcessedFrame) {
            m_fields->m_lastProcessedFrame = currentFrame;

            double frameWindow = State::get().getFrameWindowForFrame(currentFrame);
            if (frameWindow > 0.0) {
                SoundManager::get().playFrameSound(frameWindow);
            }
        }
    }
};