#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace nandl {

struct InputData {
    int index = 1;              // i
    int prevIndex = 0;          // i' (previous counted input)
    double frameWindow = 1.0;   // N_i (ticks/frames available to pass input i)
    double timePos = 0.0;       // t_i (time position of input i in seconds)
    double prevTimePos = 0.0;   // t_i' (time position of previous counted input in seconds)
};

struct ModSettings {
    bool enableNerve = false;
    double kt = 0.001;          // Nerve constant

    bool enableFatigue = false;
    double ku = 0.001;          // Fatigue constant

    bool enableCPS = false;
    double kc = 0.5;            // CPS constant

    double frameRate = 240.0;   // f (Game tick/frame rate in Hz)
    double targetHours = 24.0;  // Target completion time for L* (default 24h)
    double respawnTime = 1.0;   // Death-to-respawn delay in seconds
};

class Calculator {
public:
    static double calculateLambda(const InputData& input, const ModSettings& settings);
    static double calculatePassProbability(const InputData& input, double L, const ModSettings& settings);
    static double calculateExpectedCompletionTime(const std::vector<InputData>& inputs, double L, const ModSettings& settings);
    static double solveLStar(const std::vector<InputData>& inputs, const ModSettings& settings);
};

} // namespace nandl