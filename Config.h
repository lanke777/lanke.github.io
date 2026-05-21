#pragma once
#include <string>
#include <vector>

// 1. 定义 IMU 精度模式的参数结构
struct PrecisionMode {
    std::vector<double> gyro_bias_dph;
    std::vector<double> acc_bias_mps2;
    std::vector<double> gyro_markov_intensity_dph;
    std::vector<double> gyro_markov_correlation_time_s;
    std::vector<double> acc_markov_intensity_mps2;
    std::vector<double> acc_markov_correlation_time_s;
    std::vector<double> gyro_white_noise_dph;
    std::vector<double> acc_white_noise_mps2;
    std::vector<double> dKg_ppm_matrix;
    std::vector<double> dKa_ppm_matrix;
};

// 2. 定义单个节点的配置结构
struct NodeConfig {
    std::string name;
    std::vector<double> offset;
    int precision_mode;
    std::vector<double> initial_alignment_error;
};

// 3. 定义全局应用配置结构
struct AppConfig {
    // --- 命令行参数 ---
    int totalNodes = 0;
    int highPrecisionNodes = 0;
    std::string configFilePath;

    // --- JSON 全局参数 ---
    std::vector<double> center_pos;
    double range_noise_std_m = 0.0;
    int ranging_interval_ms = 0;
    double direction_angle_deg = 0.0;
    int imu_sample_interval_ms = 0;

    // --- IMU 精度模式配置 ---
    PrecisionMode mode1;
    PrecisionMode mode2;

    // --- 节点列表 ---
    std::vector<NodeConfig> nodes;
};