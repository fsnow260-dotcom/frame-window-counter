#include "Calculator.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

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

static double computeNaNLambda(double timeSeconds, double clickIndex, double cps) {
    constexpr double k_nerve = 0.001;
    constexpr double k_fatigue = 0.0005;
    constexpr double k_cps = 0.5;

    double l_nerve = std::exp(-k_nerve * std::max(0.0, timeSeconds));
    double l_fatigue = std::exp(-k_fatigue * std::max(0.0, clickIndex));
    
    double effCPS = std::max(1.0, cps);
    double l_cps = std::pow(4.0 / effCPS, k_cps);

    return l_nerve * l_fatigue * l_cps;
}

static double computeClickProbability(double windowSeconds, double L_star, double lambda) {
    if (windowSeconds <= 0.0) return 1e-15;

    double x = (windowSeconds * L_star * lambda) / (2.0 * M_SQRT2);
    double prob = std::erf(x);
    
    return std::clamp(prob, 1e-15, 1.0 - 1e-15);
}

CalculationResult Calculator::solve(const std::vector<InputData>& inputs, double targetProb) {
    CalculationResult res;

    if (inputs.empty()) {
        res.valid = false;
        res.errorMessage = "No click inputs available.";
        return res;
    }

    startGlobalRecalc();

    double targetLogProb = std::log(std::clamp(targetProb, 1e-6, 1.0 - 1e-6));

    std::vector<double> lambdas;
    lambdas.reserve(inputs.size());
    for (const auto& in : inputs) {
        lambdas.push_back(computeNaNLambda(in.timeSeconds, in.clickIndex, in.cps));
    }

    double low = 1e-4;
    double high = 1e7;

    for (int iter = 0; iter < 120; ++iter) {
        if (g_cancelRequested.load()) {
            res.valid = false;
            res.errorMessage = "Calculation cancelled by user.";
            return res;
        }

        double mid = low + (high - low) * 0.5;
        double currentLogProb = 0.0;

        for (size_t i = 0; i < inputs.size(); ++i) {
            double p_i = computeClickProbability(inputs[i].windowSeconds, mid, lambdas[i]);
            currentLogProb += std::log(p_i);
        }

        if (std::abs(high - low) < 1e-9 || std::abs(currentLogProb - targetLogProb) < 1e-8) {
            low = mid;
            break;
        }

        if (currentLogProb < targetLogProb) {
            low = mid;
        } else {
            high = mid;
        }
    }

    stopGlobalRecalc();

    res.L_star = low;
    res.valid = true;
    return res;
}

} // namespace nandl

void stopGlobalRecalc() { nandl::stopGlobalRecalc(); }
void startGlobalRecalc() { nandl::startGlobalRecalc(); }