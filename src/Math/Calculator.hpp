#pragma once

#include <vector>
#include <string>
#include <atomic>
#include "../Data/Types.hpp"

namespace nandl {

void stopGlobalRecalc();
void startGlobalRecalc();
bool isCalculating();

class Calculator {
public:
    static CalculationResult solve(const std::vector<InputData>& inputs, double targetProb = 0.5);
};

} // namespace nandl