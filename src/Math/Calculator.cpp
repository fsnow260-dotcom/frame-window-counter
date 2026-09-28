#include "Calculator.hpp"
#include <algorithm>
#include <limits>
#include <numeric>

namespace nandl {

static std::atomic<bool> g_isCalculating{false};
static std::atomic<bool> g_cancelRequested{false};

void stopGlobalRecalc() {
    g_cancelRequested.store(true);
    g_isCalculating.store(false);
}

// <--- THÊM HÀM NÀY --->
void startGlobalRecalc() {
    g_cancelRequested.store(false);
    g_isCalculating.store(true);
}

bool isCalculating() {
    return g_isCalculating.load();
}

// ... (Giữ nguyên phần logic calculateLambda, computeExpectedTime, solve, solveAll) ...

} // namespace nandl

// Định nghĩa hàm toàn cục ngoài namespace
void stopGlobalRecalc() {
    nandl::stopGlobalRecalc();
}

// <--- THÊM HÀM NÀY --->
void startGlobalRecalc() {
    nandl::startGlobalRecalc();
}