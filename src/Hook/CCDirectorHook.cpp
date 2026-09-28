#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include "../Data/State.hpp"

using namespace geode::prelude;

class $modify(MyDirectorHook, CCDirector) {
    void drawScene() {
        CCDirector::drawScene();

        // Hotkey trigger on '0' key press
        static bool wasPressed = false;
        bool isPressed = geode::cocos::isKeyPressed(enumKeyCodes::KEY_Zero);

        if (isPressed && !wasPressed) {
            bool newState = !State::get().isLabelVisible();
            State::get().setLabelVisible(newState);
        }
        wasPressed = isPressed;
    }
};