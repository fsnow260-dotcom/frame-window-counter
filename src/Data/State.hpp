#pragma once

#include <vector>
#include <string>
#include <mutex>
#include "Types.hpp"

class State {
private:
    State() = default;
    
    std::mutex m_mutex;
    bool m_labelVisible = true;
    double m_lStar = 0.0;
    std::vector<FrameAction> m_actions;
    std::vector<InputData> m_inputs;

public:
    static State& get() {
        static State instance;
        return instance;
    }

    bool isLabelVisible();
    void setLabelVisible(bool visible);

    double getLStar();
    void setLStar(double lStar);

    std::vector<FrameAction> getActions();
    void addAction(const FrameAction& action);
    void clearActions();

    std::vector<InputData> getInputs();
    void setInputs(const std::vector<InputData>& inputs);

    std::string getFormattedHUDString(int currentFrame);
    double getFrameWindowForFrame(int currentFrame);
};