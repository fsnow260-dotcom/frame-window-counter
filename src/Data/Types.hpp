#pragma once

#include <vector>
#include <string>

struct InputData {
    double timeSeconds = 0.0;   // Elapsed level time (t_i)
    double windowSeconds = 0.0; // Frame window size (w_i)
    double clickIndex = 0.0;    // Sequential click count (u_i)
    double cps = 0.0;           // Clicks per second at current frame
    int frame = 0;              // Game tick index
};

struct CalculationResult {
    double L_star = 0.0;
    bool valid = false;
    std::string errorMessage = "";
};

struct FrameAction {
    int frame = 0;
    double windowFrames = 0.0;
    std::string type = "";
};