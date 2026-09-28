#include "Calculator.hpp"
#include <algorithm>
#include <limits>
#include <numeric>

namespace nandl {

// Cờ kiểm soát luồng ngầm toàn cục
static std::atomic<bool> g_isCalculating{false};
static std::atomic<bool> g_cancelRequested{false};

void stopGlobalRecalc() {
    g_cancelRequested.store(true);
    g_isCalculating.store(false);
}

bool isCalculating() {
    return g_isCalculating.load();
}

// Công thức tính hệ số điều chỉnh Modifier Lambda (Nerve, Fatigue, CPS)
static double calculateLambda(
    double timeI,
    double cumInputs,
    double localCPS,
    const ModSettings& settings,
    MetricType metric
) {
    double lambdaT = std::exp(-settings.kt * timeI);                    // Nerve
    double lambdaU = std::exp(-settings.ku * cumInputs);                // Fatigue
    
    // CPS Modifier: lambda_c = (4 / max(1, 2 * cps))^kc
    double lambdaC = 1.0;
    if (localCPS > 0.0) {
        double denom = std::max(1.0, 2.0 * localCPS);
        lambdaC = std::pow(4.0 / denom, settings.kc);
    }

    switch (metric) {
        case MetricType::Nerve:        return lambdaT;
        case MetricType::Fatigue:      return lambdaU;
        case MetricType::CPS:          return lambdaC;
        case MetricType::NerveFatigue: return lambdaT * lambdaU;
        case MetricType::NerveCPS:     return lambdaT * lambdaC;
        case MetricType::FatigueCPS:   return lambdaU * lambdaC;
        case MetricType::All:          return lambdaT * lambdaU * lambdaC;
        case MetricType::Base:
        default:                       return 1.0;
    }
}

double Calculator::computeExpectedTime(
    const std::vector<InputData>& inputs,
    const ModSettings& settings,
    MetricType metric,
    double L,
    double* outPassProb,
    double* outAttempts
) {
    if (inputs.empty() || settings.tps <= 0.0 || L <= 0.0) {
        if (outPassProb) *outPassProb = 0.0;
        if (outAttempts) *outAttempts = 0.0;
        return std::numeric_limits<double>::infinity();
    }

    // Lọc danh sách các input đang active
    std::vector<InputData> activeInputs;
    activeInputs.reserve(inputs.size());
    for (const auto& in : inputs) {
        if (in.active && in.window > 0.0) {
            activeInputs.push_back(in);
        }
    }

    if (activeInputs.empty()) {
        if (outPassProb) *outPassProb = 1.0;
        if (outAttempts) *outAttempts = 1.0;
        return 0.0;
    }

    const size_t N = activeInputs.size();
    std::vector<double> p(N, 1.0);

    double cumulativeInputs = 0.0;
    const double sqrt2 = 1.41421356237309504880;

    for (size_t i = 0; i < N; ++i) {
        cumulativeInputs += activeInputs[i].count;

        // Tính Local CPS
        double localCPS = 0.0;
        if (i > 0) {
            double dt = activeInputs[i].time - activeInputs[i - 1].time;
            if (dt > 1e-6) {
                localCPS = static_cast<double>(activeInputs[i].count) / dt;
            }
        }

        // Tính hệ số Lambda
        double lambda = calculateLambda(
            activeInputs[i].time,
            cumulativeInputs,
            localCPS,
            settings,
            metric
        );

        // Công thức NaNDL:
        // Window time: w_i = N_i / TPS
        // Base Sigma: s_i = 0.5 * w_i * L
        // Modded Sigma: s_i_mod = s_i * lambda
        double w_i = activeInputs[i].window / settings.tps;
        double sigma = 0.5 * w_i * L * lambda;

        // Pass prob p_i = erf(sigma / sqrt(2))
        double prob = std::erf(sigma / sqrt2);
        
        // Khống chế biên để tránh tràn số học
        p[i] = std::clamp(prob, 1e-15, 1.0 - 1e-15);
    }

    // Tính Xác suất hoàn thành toàn bộ level P(C) bằng Log-space
    double logPassProb = 0.0;
    for (double pi : p) {
        logPassProb += std::log(pi);
    }
    double passProb = std::exp(logPassProb);

    // Tính Thời gian thử kỳ vọng E[T_A]
    double expectedAttemptDuration = 0.0;
    double currentReachProb = 1.0;

    for (size_t i = 0; i < N; ++i) {
        double failProb = 1.0 - p[i];
        // Thời gian nếu chết ở input i: t_i + respawnTime
        double deathTime = activeInputs[i].time + settings.respawnTime;
        
        expectedAttemptDuration += deathTime * currentReachProb * failProb;
        currentReachProb *= p[i];
    }

    // Thời gian nếu hoàn thành level
    double totalLevelTime = activeInputs.back().time;
    expectedAttemptDuration += totalLevelTime * passProb;

    // Thời gian hoàn thành kỳ vọng E[T_C] = E[T_A] / P(C)
    double expectedCompletionTime = (passProb > 1e-300) 
        ? (expectedAttemptDuration / passProb) 
        : std::numeric_limits<double>::infinity();

    if (outPassProb) *outPassProb = passProb;
    if (outAttempts) *outAttempts = (passProb > 1e-300) ? (1.0 / passProb) : std::numeric_limits<double>::infinity();

    return expectedCompletionTime;
}

CalculationResult Calculator::solve(
    const std::vector<InputData>& inputs,
    const ModSettings& settings,
    MetricType metric
) {
    CalculationResult res;
    g_isCalculating.store(true);

    double targetTime = settings.targetSolveTime;
    
    // Thuật toán Binary Search tìm L* sao cho E[T_C](L*) = targetSolveTime (24h)
    double low = 1e-7;
    double high = 100000.0; // Giới hạn trên cao đủ đáp ứng cả các level cực khó (Silent Circles, Carmine Bullet)

    // Mở rộng ranh giới upper bound nếu level quá khó
    if (computeExpectedTime(inputs, settings, metric, high) > targetTime) {
        high = 1e8;
    }

    for (int iter = 0; iter < 60; ++iter) {
        if (g_cancelRequested.load()) {
            res.valid = false;
            g_isCalculating.store(false);
            return res;
        }

        double mid = low + (high - low) / 2.0;
        double currentTC = computeExpectedTime(inputs, settings, metric, mid);

        if (currentTC > targetTime) {
            low = mid;  // E[T_C] còn quá lớn -> L* quá nhỏ -> Tăng L*
        } else {
            high = mid; // E[T_C] đã nhỏ hơn target -> Giảm L*
        }

        if ((high - low) < 1e-8) break;
    }

    res.lStar = (low + high) / 2.0;
    res.expectedTime = computeExpectedTime(
        inputs, settings, metric, res.lStar, 
        &res.passProbability, &res.expectedAttempts
    );
    res.valid = true;

    g_isCalculating.store(false);
    return res;
}

std::array<CalculationResult, 8> Calculator::solveAll(
    const std::vector<InputData>& inputs,
    const ModSettings& settings
) {
    std::array<CalculationResult, 8> results;
    g_cancelRequested.store(false);

    for (int i = 0; i < 8; ++i) {
        if (g_cancelRequested.load()) break;
        results[i] = solve(inputs, settings, static_cast<MetricType>(i));
    }

    return results;
}

} // namespace nandl

// Định nghĩa hàm toàn cục bên ngoài namespace cho State.cpp
void stopGlobalRecalc() {
    nandl::stopGlobalRecalc();
}