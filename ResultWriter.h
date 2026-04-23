#pragma once

#include "DataTypes.h"
#include <vector>

using namespace std;

/// @brief 结果输出器，负责统计误差并生成报告
class ResultWriter {
public:
    /// @brief 评估平差结果，计算 RMS，并生成 detailed_results.csv 和 txt 报告
    /// @param results 平差后的结果记录
    static void evaluateAndSaveResults(const vector<ResultRecord>& results);
};
