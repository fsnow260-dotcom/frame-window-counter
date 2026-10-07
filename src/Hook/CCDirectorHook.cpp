#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include "../Data/State.hpp"

using namespace geode::prelude;

class $modify(MyKeyboardHook, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat) {
        if (down && !repeat && key == KEY_Zero) {
            bool currentState = State::get().isLabelVisible();
            State::get().setLabelVisible(!currentState);
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);
    }
};