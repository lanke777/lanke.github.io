#include "AdjustmentEngine.h"
#include "Config.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace Eigen;
using namespace std;


//内部数学工具的具体实现


ObsFactor AdjustmentEngine::processRanging(const Node& nm, const Node& ns, const Observation& obs, double t_ref) {
    ObsFactor factor;
    factor.is_ranging = true;
    factor.idx_i = nm.matrix_idx;
    factor.idx_j = ns.matrix_idx;

    // 1. 时间归算推算坐标
    Vector3d pm = nm.predictPos(obs.timestamp, t_ref);
    Vector3d ps = ns.predictPos(obs.timestamp, t_ref);

    // 2. 计算计算值与方向向量
    Vector3d diff = ps - pm;
    double dist_cal = diff.norm();

    // 异常保护：防止重叠导致除以0
    if (dist_cal < 1e-5) {
        factor.is_valid = false;
        return factor;
    }

    // 3. 计算方向余弦、残差与权值
    factor.u_vec = diff / dist_cal;
    factor.residual = obs.value.x() - dist_cal;
    factor.weight = obs.getWeightMatrix(SIGMA_0)(0, 0);
    factor.is_valid = true;

    return factor;
}

ObsFactor AdjustmentEngine::processIns(const Node& node, const Observation& obs, double t_ref) {
    ObsFactor factor;
    factor.is_ranging = false;
    factor.idx_i = node.matrix_idx;
    factor.idx_j = -1;

    Vector3d pos_pred = node.predictPos(obs.timestamp, t_ref);
    factor.residual_vec = obs.value - pos_pred;
    factor.weight_mat = obs.getWeightMatrix(SIGMA_0);
    factor.is_valid = true;

    return factor;
}

void AdjustmentEngine::accumulate(MatrixXd& N, VectorXd& U, const ObsFactor& factor) {
    if (!factor.is_valid) return;

    if (factor.is_ranging) {
        int i = factor.idx_i;
        int j = factor.idx_j;
        double p = factor.weight;

        // 构建 M 矩阵 (方向余弦外积)
        Matrix3d M = factor.u_vec * factor.u_vec.transpose();
        Matrix3d pM = p * M;

        // 累加 N 矩阵
        if (i >= 0) N.block(i, i, 3, 3) += pM;
        if (j >= 0) N.block(j, j, 3, 3) += pM;

        if (i >= 0 && j >= 0) {
            N.block(i, j, 3, 3) -= pM;
            N.block(j, i, 3, 3) -= pM;
        }

        // 累加 U 向量
        Vector3d w_l_u = p * factor.residual * factor.u_vec;
        if (i >= 0) U.segment(i, 3) -= w_l_u;
        if (j >= 0) U.segment(j, 3) += w_l_u;
    }
    else {
        int i = factor.idx_i;
        if (i >= 0) {
            N.block(i, i, 3, 3) += factor.weight_mat;
            U.segment(i, 3) += factor.weight_mat * factor.residual_vec;
        }
    }
}
map<int, Node> AdjustmentEngine::solveInterval(double t_ref, const vector<Observation>& obs_list, map<int, Node>& nodes) {
    // 如果没有观测数据或者节点为空，直接返回当前状态
    if (obs_list.empty() || nodes.empty()) return nodes;

    int num_nodes = 0;
    for (auto& kv : nodes) {
        kv.second.matrix_idx = num_nodes * 3;
        num_nodes++;
    }
    int dim = num_nodes * 3;

    const int MAX_ITER = 10;
    const double CONVERGENCE_TOL = 1e-4;

    for (int iter = 0; iter < MAX_ITER; ++iter) {
        MatrixXd N = MatrixXd::Zero(dim, dim);
        VectorXd U = VectorXd::Zero(dim);

        for (const auto& obs : obs_list) {
            ObsFactor factor;
            if (obs.type == OBS_INS) {
                if (nodes.find(obs.master_id) != nodes.end()) {
                    factor = processIns(nodes.at(obs.master_id), obs, t_ref);
                }
            }
            else if (obs.type == OBS_RANGING) {
                if (nodes.find(obs.master_id) != nodes.end() && nodes.find(obs.slave_id) != nodes.end()) {
                    factor = processRanging(nodes.at(obs.master_id), nodes.at(obs.slave_id), obs, t_ref);
                }
            }
            accumulate(N, U, factor);
        }

        VectorXd dx = N.colPivHouseholderQr().solve(U);

        double max_dx = 0.0;
        for (auto& kv : nodes) {
            int idx = kv.second.matrix_idx;
            Vector3d dpos = dx.segment(idx, 3);
            kv.second.pos_est += dpos;
            max_dx = max(max_dx, dpos.norm());
        }

        if (max_dx < CONVERGENCE_TOL) {
            break;
        }
    }

    return nodes;
}