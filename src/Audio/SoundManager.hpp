#pragma once

#include <Geode/Geode.hpp>

class SoundManager {
public:
    static SoundManager& get() {
        static SoundManager instance;
        return instance;
    }

    void init() {
        geode::FMODAudioEngine::sharedEngine()->preloadEffect("ding.ogg"_spr);
    }

    void playFrameSound(double frameWindow) {
        if (frameWindow >= 1.0) {
            geode::FMODAudioEngine::sharedEngine()->playEffect("ding.ogg"_spr);
        }
    }

    void playDingSound() {
        geode::FMODAudioEngine::sharedEngine()->playEffect("ding.ogg"_spr);
    }
};