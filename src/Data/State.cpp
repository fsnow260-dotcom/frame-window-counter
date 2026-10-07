#include "State.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

bool State::isLabelVisible() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_labelVisible;
}

void State::setLabelVisible(bool visible) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_labelVisible = visible;
}

double State::getLStar() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lStar;
}

void State::setLStar(double lStar) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_lStar = lStar;
}

std::vector<FrameAction> State::getActions() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_actions;
}

void State::addAction(const FrameAction& action) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_actions.push_back(action);
}

void State::clearActions() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_actions.clear();
}

std::vector<InputData> State::getInputs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_inputs;
}

void State::setInputs(const std::vector<InputData>& inputs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_inputs = inputs;
}

std::string State::getFormattedHUDString(int currentFrame) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::stringstream ss;
    ss << "Frame: " << currentFrame;

    auto it = std::find_if(m_actions.begin(), m_actions.end(), [currentFrame](const FrameAction& act) {
        return act.frame == currentFrame;
    });

    if (it != m_actions.end()) {
        ss << " | Window: " << std::fixed << std::setprecision(1) << it->windowFrames << "f";
    }

    if (m_lStar > 0.0) {
        ss << " | L*: " << std::fixed << std::setprecision(3) << m_lStar;
    }

    return ss.str();
}

double State::getFrameWindowForFrame(int currentFrame) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& act : m_actions) {
        if (act.frame == currentFrame) {
            return act.windowFrames;
        }
    }
    return 0.0;
}