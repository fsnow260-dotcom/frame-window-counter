#pragma once

#include <vector>
#include <string>
#include <array>
#include <cmath>
#include <atomic>
#include <mutex>

namespace nandl {

// 8 biến thể chỉ số kết hợp (Base, Nerve, Fatigue, CPS và các tổ hợp)
enum class MetricType {
    Base = 0,
    Nerve,
    Fatigue,
    CPS,
    NerveFatigue,
    NerveCPS,
    FatigueCPS,
    All
};

// Cấu trúc dữ liệu cho mỗi Input/Frame Action
struct InputData {
    double time = 0.0;           // Thời điểm bấm nút (giây)
    double window = 0.0;         // Frame window (số frame/tick cho phép N_i)
    int count = 1;               // Số input trong frame (Sub-frame / I/F)
    bool active = true;          // Bật/tắt tính toán cho input này
};

// Thiết lập thông số mô hình NaNDL
struct ModSettings {
    double tps = 240.0;               // Tần số tính toán (TPS / Macro FPS)
    double respawnTime = 1.0;         // Thời gian hồi sinh sau khi chết (giây)
    double targetSolveTime = 86400.0; // Thời gian hoàn thành mục tiêu (mặc định 24h = 86400s)
    
    // Hằng số mô hình NaNDL
    double kt = 0.01;            // Hệ số giảm sức ép tâm lý (Nerve decay)
    double ku = 0.001;           // Hệ số mệt mỏi theo số lần bấm (Fatigue decay)
    double kc = 0.5;             // Hệ số phạt tốc độ bấm nhanh (CPS penalty)
};

// Kết quả tính toán cho từng chỉ số
struct CalculationResult {
    double lStar = 0.0;               // Giá trị độ chính xác L*
    double passProbability = 0.0;      // Xác suất qua level P(C)
    double expectedAttempts = 0.0;     // Số lần thử kỳ vọng E[A]
    double expectedTime = 0.0;         // Thời gian hoàn thành kỳ vọng E[T_C]
    bool valid = false;
};

// Lớp xử lý thuật toán NaNDL
class Calculator {
public:
    // Tính toán chỉ số L* cho 1 biến thể cụ thể
    static CalculationResult solve(
        const std::vector<InputData>& inputs,
        const ModSettings& settings,
        MetricType metric
    );

    // Tính toán đồng thời cả 8 biến thể
    static std::array<CalculationResult, 8> solveAll(
        const std::vector<InputData>& inputs,
        const ModSettings& settings
    );

    // Hàm tính Thời gian kỳ vọng E[T_C] theo giá trị L truyền vào
    static double computeExpectedTime(
        const std::vector<InputData>& inputs,
        const ModSettings& settings,
        MetricType metric,
        double L,
        double* outPassProb = nullptr,
        double* outAttempts = nullptr
    );
};

// Các hàm điều khiển luồng tính toán toàn cục (Tránh lỗi linker/compiler)
void stopGlobalRecalc();
bool isCalculating();

} // namespace nandl

// Khai báo ngoài namespace toàn cục phòng trường hợp State.cpp gọi trực tiếp
void stopGlobalRecalc();