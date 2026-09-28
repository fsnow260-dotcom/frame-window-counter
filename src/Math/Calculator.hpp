#pragma once

#include <vector>
#include <string>
#include <array>
#include <cmath>
#include <atomic>
#include <mutex>
#include "../Data/Types.hpp"

namespace nandl {

// Global calculation control flags
void stopGlobalRecalc();
void startGlobalRecalc();
bool isCalculating();

class Calculator {
public:
    static CalculationResult solve(const std::vector<InputData>& inputs, double targetProb = 0.5);
};

} // namespace nandl

// Global C-style declarations for UI callers
void stopGlobalRecalc();
void startGlobalRecalc();