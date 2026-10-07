#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class SoundManager {
public:
    static SoundManager& get() {
        static SoundManager instance;
        return instance;
    }

    void init() {
        FMODAudioEngine::sharedEngine()->preloadEffect("ding.ogg"_spr);
    }

    void playFrameSound(double frameWindow) {
        if (frameWindow >= 1.0) {
            FMODAudioEngine::sharedEngine()->playEffect("ding.ogg"_spr);
        }
    }

    void playDingSound() {
        FMODAudioEngine::sharedEngine()->playEffect("ding.ogg"_spr);
    }
};