#pragma once

#include <vector>
#include <string>
#include <array>
#include <cmath>
#include <atomic>
#include <mutex>

namespace nandl {

// ... (Giữ nguyên các enum MetricType, InputData, ModSettings, CalculationResult, Calculator) ...

// Các hàm điều khiển luồng tính toán toàn cục
void stopGlobalRecalc();
void startGlobalRecalc(); // <--- THÊM DÒNG NÀY
bool isCalculating();

} // namespace nandl

// Khai báo ngoài namespace toàn cục cho các file UI gọi trực tiếp
void stopGlobalRecalc();
void startGlobalRecalc(); // <--- THÊM DÒNG NÀY