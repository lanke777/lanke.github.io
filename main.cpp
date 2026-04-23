#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <iomanip>
#include "Config.h"
#include "Helper.h"
#include "DataTypes.h"
#include "DataLoader.h"
#include "AdjustmentEngine.h"
#include "ResultWriter.h"

using namespace std;
using namespace Eigen;


double SIGMA_0 = 30.0;           // 基准标准差
double SIGMA_RANGE = 35.0;       // 测距误差
double SIGMA_POS_LOW = 1000.0;   // 低精度惯导误差
double SIGMA_POS_HIGH = 50;     // 高精度惯导误差
int N_NODE = 4; //总节点数
double  STAGE1_SEC = 240.0; //平飞段开始时间,也即直飞段时长 (秒)
string FILE_PREFIX = "12(0)"; //输入输出文件前缀 X(Y)，其中 X 是节点总数，Y 是高精度点数量
std::set<int> KNOWN_NODE_IDS = { };



int main() {





    string filename = ".//" + FILE_PREFIX + "//" + FILE_PREFIX + "_obs.csv";
    vector<SimDataRow> dataset;

    // 加载数据
    cout << "[启动] 正在读取文件..." << endl;
    if (!DataLoader::loadGatherFile(filename, dataset)) return -1;

    

    // 按照时间间隔分组 (比如每 1.0 秒解算一次)
    double interval_duration = 1.0;
    map<double, vector<SimDataRow>> interval_groups;

    for (const auto& row : dataset) {
        int interval_idx = (int)floor(row.timestamp / interval_duration);
        double t_ref = interval_idx * interval_duration;
        interval_groups[t_ref].push_back(row);
    }

    // 定义全局节点
    map<int, Node> global_nodes;
    vector<ResultRecord> global_results;

    // 按照时间顺序处理
    int i = 0;
    for (const auto& kv : interval_groups) {
        double t_ref = kv.first;
        const vector<SimDataRow>& interval_data = kv.second;

        if(i++ % 60  == 0) cout << "*";


        // 准备本时间段的观测值数据
        vector<Observation> current_obs;
        Helper::prepareIntervalData(interval_data, global_nodes, current_obs);

        // 进行平差解算，并接收返回的字典
        map<int, Node> adjusted_nodes = AdjustmentEngine::solveInterval(t_ref, current_obs, global_nodes);

        //遍历拿到的 adjusted_nodes，提取结果保存
        for (const auto& node_kv : adjusted_nodes) {
            int node_id = node_kv.first;
            Vector3d est_pos = node_kv.second.pos_est;

            // 在外部的历元数据中搜寻真值和初始值
            Vector3d truth_pos = Vector3d::Zero();
            Vector3d raw_sim_pos = Vector3d::Zero();
            for (const auto& row : interval_data) {
                if (row.m_id == node_id) { truth_pos = Vector3d(row.m_true.X, row.m_true.Y, row.m_true.Z); raw_sim_pos = Vector3d(row.m_sim.X, row.m_sim.Y, row.m_sim.Z); break; }
                if (row.s_id == node_id) { truth_pos = Vector3d(row.s_true.X, row.s_true.Y, row.s_true.Z); raw_sim_pos = Vector3d(row.s_sim.X, row.s_sim.Y, row.s_sim.Z); break; }
            }

            ResultRecord rec;
            rec.t_ref = t_ref;
            rec.node_id = node_id;
            rec.pos_sim = raw_sim_pos;
            rec.pos_est = est_pos;
            rec.pos_true = truth_pos;
            global_results.push_back(rec);

            if (0) {
                cout <<t_ref ;
                // 直接在屏幕上输出结果：
                cout << setprecision(3) << "  节点 ID: " << node_id << " | 平差前误差: " << (raw_sim_pos - truth_pos).norm()
                    << " m -> 平差后误差: " << (est_pos - truth_pos).norm() << " m\n";
            }
        }
    }

    // 保存最终的综合精度评估
    ResultWriter::evaluateAndSaveResults(global_results);

    return 0;
}