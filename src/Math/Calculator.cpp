#include "Calculator.hpp"

namespace nandl {

double Calculator::calculateLambda(const InputData& input, const ModSettings& settings) {
    double lambda = 1.0;

    // 1. Nerve Multiplier: lambda_{t,i} = e^(-k_t * t_i)
    if (settings.enableNerve && settings.kt > 0.0) {
        lambda *= std::exp(-settings.kt * input.timePos);
    }

    // 2. Fatigue Multiplier: lambda_{u,i} = e^(-k_u * i)
    if (settings.enableFatigue && settings.ku > 0.0) {
        lambda *= std::exp(-settings.ku * static_cast<double>(input.index));
    }

    // 3. CPS Multiplier: c_i = (i - i') / (t_i - t_i'), lambda_{c,i} = (4 / max(1, 2 * c_i))^k_c
    // Only apply if there is a valid previous input (prevIndex > 0)
    if (settings.enableCPS && settings.kc > 0.0 && input.prevIndex > 0) {
        double deltaTime = input.timePos - input.prevTimePos;
        if (deltaTime > 1e-5) {
            double deltaInputs = static_cast<double>(input.index - input.prevIndex);
            double localCPS = deltaInputs / deltaTime;
            double cpsDenominator = std::max(1.0, 2.0 * localCPS);
            lambda *= std::pow(4.0 / cpsDenominator, settings.kc);
        }
    }

    return lambda;
}

double Calculator::calculatePassProbability(const InputData& input, double L, const ModSettings& settings) {
    double w_i = input.frameWindow * (1.0 / settings.frameRate);
    double s_i = 0.5 * w_i * L;

    double lambda = calculateLambda(input, settings);
    double moddedSigma = s_i * lambda;

    // erf input bounded to avoid numerical instability
    double arg = moddedSigma / std::sqrt(2.0);
    double prob = std::erf(std::min(arg, 5.0));
    
    // Clamp probability slightly below 1.0 and above 0 to prevent exact 0 or 1 edge cases
    return std::clamp(prob, 1e-12, 1.0 - 1e-15);
}

double Calculator::calculateExpectedCompletionTime(const std::vector<InputData>& inputs, double L, const ModSettings& settings) {
    if (inputs.empty()) return 0.0;

    size_t n = inputs.size();
    std::vector<double> p(n);
    std::vector<double> q(n);
    std::vector<double> r(n);

    // Step 1: Compute p_i and q_i
    for (size_t i = 0; i < n; ++i) {
        p[i] = calculatePassProbability(inputs[i], L, settings);
        q[i] = 1.0 - p[i];
    }

    // Step 2: Compute reaching probabilities r_i
    r[0] = 1.0;
    for (size_t i = 1; i < n; ++i) {
        r[i] = r[i - 1] * p[i - 1];
        if (r[i] < 1e-300) r[i] = 1e-300; // Underflow protection
    }

    // Step 3: Compute total completion probability P(C)
    double PC = r[n - 1] * p[n - 1];
    if (PC <= 1e-300) {
        return 1e18; // Return an arbitrarily large duration if completion probability is 0
    }

    // Step 4: Compute E[T_A] including respawn time penalty on failure
    double t_n = inputs.back().timePos;
    double failureTimeSum = 0.0;

    for (size_t i = 0; i < n; ++i) {
        failureTimeSum += (inputs[i].timePos + settings.respawnTime) * r[i] * q[i];
    }

    double ETA = (t_n * PC) + failureTimeSum;

    // Step 5: E[T_C] = E[T_A] / P(C)
    return ETA / PC;
}

double Calculator::solveLStar(const std::vector<InputData>& inputs, const ModSettings& settings) {
    if (inputs.empty()) return 0.0;

    double targetSeconds = settings.targetHours * 3600.0;

    // Binary search bounds (expanded high bound for extreme levels)
    double low = 0.0001;
    double high = 10000.0;
    double mid = 0.0;

    for (int iter = 0; iter < 70; ++iter) {
        mid = low + (high - low) / 2.0;
        double ETC = calculateExpectedCompletionTime(inputs, mid, settings);

        if (ETC > targetSeconds) {
            low = mid;
        } else {
            high = mid;
        }
    }

    return mid;
}

} // namespace nandl