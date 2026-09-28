#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../Audio/SoundManager.hpp"
#include "../Data/State.hpp"
#include "../Math/Calculator.hpp"

using namespace geode::prelude;

class $modify(MyPlayLayerHook, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_hudLabel = nullptr;
        int m_lastProcessedFrame = -1;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Initialize audio engine instance
        SoundManager::get().init();

        // Create UI overlay label on m_uiLayer
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

        // Respect visibility toggle state
        if (!State::get().isLabelVisible()) {
            m_fields->m_hudLabel->setVisible(false);
            return;
        }

        m_fields->m_hudLabel->setVisible(true);

        int currentFrame = static_cast<int>(this->m_gameState.m_currentPoint.x);

        // Update live HUD text from loaded macro state
        std::string hudText = State::get().getFormattedHUDString(currentFrame);
        if (hudText.empty()) {
            m_fields->m_hudLabel->setString("Frame Counter: Ready (No Macro)");
        } else {
            m_fields->m_hudLabel->setString(hudText.c_str());
        }

        // Process active frame hits
        if (currentFrame != m_fields->m_lastProcessedFrame) {
            m_fields->m_lastProcessedFrame = currentFrame;

            double frameWindow = State::get().getFrameWindowForFrame(currentFrame);

            if (frameWindow > 0.0) {
                // Trigger SoundManager for tight windows (1f to 5f)
                if (frameWindow >= 1.0 && frameWindow <= 5.0) {
                    SoundManager::get().playFrameSound(frameWindow);
                }
            }
        }
    }
};