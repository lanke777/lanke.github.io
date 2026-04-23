#include "Helper.h"
#include "Config.h" 

using namespace std;
using namespace Eigen;

void Helper::prepareIntervalData (const vector<SimDataRow>& interval_data,
    map<int, Node>& global_nodes,
    vector<Observation>& obs_list) {
    obs_list.clear();

    for (const auto& row : interval_data) {
        // 1. 注册与更新节点状态 (存入 velocity_curve)
        if (global_nodes.find(row.m_id) == global_nodes.end()) {
            global_nodes[row.m_id] = Node(row.m_id);
            global_nodes[row.m_id].pos_est = Vector3d(row.m_sim.X, row.m_sim.Y, row.m_sim.Z);
        }
        global_nodes[row.m_id].velocity_curve[row.timestamp] = Vector3d(row.m_sim.vx, row.m_sim.vy, row.m_sim.vz);

        if (row.s_id != -1) {
            if (global_nodes.find(row.s_id) == global_nodes.end()) {
                global_nodes[row.s_id] = Node(row.s_id);
                global_nodes[row.s_id].pos_est = Vector3d(row.s_sim.X, row.s_sim.Y, row.s_sim.Z);
            }
            global_nodes[row.s_id].velocity_curve[row.timestamp] = Vector3d(row.s_sim.vx, row.s_sim.vy, row.s_sim.vz);
        }

        // 2. 生成标准的 Observation 观测值
        double sigma_m = isHighPrecision(row.m_id) ? SIGMA_POS_HIGH : SIGMA_POS_LOW;
        obs_list.push_back(Observation(row.timestamp, row.m_id,
            Vector3d(row.m_sim.X, row.m_sim.Y, row.m_sim.Z), 
            Vector3d(row.m_sim.vx, row.m_sim.vy, row.m_sim.vz),
            Vector3d::Constant(sigma_m)));

        if (row.s_id != -1) {
            double sigma_s = isHighPrecision(row.s_id) ? SIGMA_POS_HIGH : SIGMA_POS_LOW;
            obs_list.push_back(Observation(row.timestamp, row.s_id,
                Vector3d(row.s_sim.X, row.s_sim.Y, row.s_sim.Z), 
                Vector3d(row.s_sim.vx, row.s_sim.vy, row.s_sim.vz),
                Vector3d::Constant(sigma_s)));
        }

        if (row.range_sim > 0.1) {
            obs_list.push_back(Observation(row.timestamp, row.m_id, row.s_id,
                row.range_sim, Vector3d::Zero(), SIGMA_RANGE));
        }
    }
}