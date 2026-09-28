#include "Calculator.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace nandl {

static std::atomic<bool> g_isCalculating{false};
static std::atomic<bool> g_cancelRequested{false};

void stopGlobalRecalc() {
    g_cancelRequested.store(true);
    g_isCalculating.store(false);
}

void startGlobalRecalc() {
    g_cancelRequested.store(false);
    g_isCalculating.store(true);
}

bool isCalculating() {
    return g_isCalculating.load();
}

static double computePassProbability(double windowSeconds, double L_star, double lambda) {
    if (windowSeconds <= 0.0) return 1e-15;
    double sigma = 0.5 * windowSeconds * L_star * lambda;
    double prob = std::erf(sigma / M_SQRT2);
    return std::clamp(prob, 1e-15, 1.0 - 1e-15);
}

static double computeLambda(double timeSeconds, double clickIndex, double cps) {
    constexpr double k_t = 0.001;  // Nerve decay factor
    constexpr double k_u = 0.0005; // Fatigue factor
    constexpr double k_c = 0.5;    // CPS factor

    double lambda_nerve = std::exp(-k_t * timeSeconds);
    double lambda_fatigue = std::exp(-k_u * clickIndex);
    double effectiveCPS = std::max(1.0, 2.0 * cps);
    double lambda_cps = std::pow(4.0 / effectiveCPS, k_c);

    return lambda_nerve * lambda_fatigue * lambda_cps;
}

CalculationResult Calculator::solve(const std::vector<InputData>& inputs, double targetProb) {
    CalculationResult res;
    if (inputs.empty()) {
        res.valid = false;
        return res;
    }

    double low = 0.001;
    double high = 1e8;
    double targetLogProb = std::log(targetProb);

    for (int iter = 0; iter < 100; ++iter) {
        double mid = low + (high - low) / 2.0;
        double currentLogProb = 0.0;

        for (const auto& input : inputs) {
            if (g_cancelRequested.load()) {
                res.valid = false;
                return res;
            }
            double lambda = computeLambda(input.timeSeconds, input.clickIndex, input.cps);
            double p_i = computePassProbability(input.windowSeconds, mid, lambda);
            currentLogProb += std::log(p_i);
        }

        if (std::abs(currentLogProb - targetLogProb) < 1e-6) {
            low = mid;
            break;
        }

        if (currentLogProb < targetLogProb) {
            low = mid;
        } else {
            high = mid;
        }
    }

    res.L_star = low;
    res.valid = true;
    return res;
}

} // namespace nandl

void stopGlobalRecalc() { nandl::stopGlobalRecalc(); }
void startGlobalRecalc() { nandl::startGlobalRecalc(); }