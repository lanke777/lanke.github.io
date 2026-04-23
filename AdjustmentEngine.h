#pragma once

#include "DataTypes.h"
#include <vector>
#include <map>

/// @brief 核心平差引擎，负责单时间间隔的最小二乘解算
class AdjustmentEngine {
public:
    /// @brief 对单个间隔的数据进行最小二乘平差解算
    /// @param t_ref 间隔的参考时间 (秒)
    /// @param obs_list 该间隔内的所有观测数据
    /// @param nodes 该间隔内的所有节点
    /// @return 包含该历元所有节点平差后状态的字典 (Key为节点ID)
    static std::map<int, Node> solveInterval(double t_ref, const vector<Observation>& obs_list, map<int, Node>& nodes);

private:

    /// @brief 处理测距观测值，生成线性化因子
    static ObsFactor processRanging(const Node& nm, const Node& ns, const Observation& obs, double t_ref);

    /// @brief 处理惯导位置观测值，生成线性化因子
    static ObsFactor processIns(const Node& node, const Observation& obs, double t_ref);

    /// @brief 将线性化因子累加到全局法方程 (N矩阵和U向量) 中
    static void accumulate(Eigen::MatrixXd& N, Eigen::VectorXd& U, const ObsFactor& factor);
};