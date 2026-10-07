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
    /// Solves L* using the exact NaN GD probability model:
    ///   P_total(L*) = \prod_i erf( (w_i * L* * \lambda_i) / (2 * sqrt(2)) )
    /// Target probability defaults to 0.5 (50% pass probability).
    static CalculationResult solve(const std::vector<InputData>& inputs, double targetProb = 0.5);
};

} // namespace nandl

void stopGlobalRecalc();
void startGlobalRecalc();