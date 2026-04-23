#include "ResultWriter.h"
#include "Config.h" // 需要使用 isHighPrecision(node_id)
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <map>
#include <iomanip>

using namespace std;
using namespace Eigen;

// --- 辅助结构体：用于自动累加和计算特定分组的 RMS ---
struct RmsStats {
    double sq_sim_x = 0, sq_sim_y = 0, sq_sim_z = 0;
    double sq_est_x = 0, sq_est_y = 0, sq_est_z = 0;
    int count = 0;

    // 累加误差平方
    void add(const Vector3d& err_sim, const Vector3d& err_est) {
        sq_sim_x += err_sim.x() * err_sim.x();//平差前
        sq_sim_y += err_sim.y() * err_sim.y();//平差前
        sq_sim_z += err_sim.z() * err_sim.z();//平差前
        

        sq_est_x += err_est.x() * err_est.x();//平差后
        sq_est_y += err_est.y() * err_est.y();//平差后
        sq_est_z += err_est.z() * err_est.z();//平差后

        count++;
    }

    // 格式化输出报告块
    void printReportBlock(ofstream& out, const string& title) const {
        if (count == 0) {
            out << "--- " << title << " (无数据) ---\n\n";
            return;
        }

        // 计算各个方向的 RMS
        double r_sim_x = sqrt(sq_sim_x / count);
        double r_sim_y = sqrt(sq_sim_y / count);
        double r_sim_z = sqrt(sq_sim_z / count);
        double r_sim_3d = sqrt((sq_sim_x + sq_sim_y + sq_sim_z) / count);

        double r_est_x = sqrt(sq_est_x / count);
        double r_est_y = sqrt(sq_est_y / count);
        double r_est_z = sqrt(sq_est_z / count);
        double r_est_3d = sqrt((sq_est_x + sq_est_y + sq_est_z) / count);

        auto calcImp = [](double pre, double post) {
            return pre > 1e-6 ? (pre - post) / pre * 100.0 : 0.0;
        };

        out << "--- " << title << " (样本数 N=" << count << ") ---\n";
        out << "  平差前RMS(m) : X=" << setw(7) << setprecision(4) << r_sim_x << ",Y=" << setw(7) << r_sim_y
            << ", Z=" << setw(7) << r_sim_z << ", 3D=" << setw(7) << r_sim_3d << "\n";
        out << "  平差后RMS(m) : X=" << setw(7) << setprecision(4) << r_est_x << ", Y=" << setw(7) << r_est_y
            << ", Z=" << setw(7) << r_est_z << ", 3D=" << setw(7) << r_est_3d << "\n";
        out << "  RMS提升比例: X=" << setw(6) <<setprecision(1) <<calcImp(r_sim_x, r_est_x) << "%, Y="
            << setw(6) << calcImp(r_sim_y, r_est_y) << "%, Z="
            << setw(6) << calcImp(r_sim_z, r_est_z) << "%, 3D="
            << setw(6) << calcImp(r_sim_3d, r_est_3d) << "%\n\n";
    }
};

void ResultWriter::evaluateAndSaveResults(const vector<ResultRecord>& results) {
    if (results.empty()) return;

    // 1. 打开详细结果 CSV，新增了 NodeType 列
    ofstream f_detail("detailed_results.csv");
    f_detail << "Time,NodeID,NodeType,Sim_X,Sim_Y,Sim_Z,Est_X,Est_Y,Est_Z,True_X,True_Y,True_Z,Error_Sim_3D,Error_Est_3D\n";

    // 2. 创建统计分组
    map<double, RmsStats> time_stats_high; // 按时间窗口分组的高精度点统计
    map<double, RmsStats> time_stats_low;  // 按时间窗口分组的低精度点统计

    RmsStats total_stats_high;             // 全局高精度点统计
    RmsStats total_stats_low;              // 全局低精度点统计

    RmsStats total_stats_high_2;             //平飞段全局高精度点统计
    RmsStats total_stats_low_2;              //平飞段全局低精度点统计

    // 为了保证输出时时间是按顺序的，用 set 提取所有出现过的时间戳
    set<double> time_windows;

    int i = 0; //历元计数
    for (const auto& r : results) {
        time_windows.insert(r.t_ref);

        Vector3d err_sim = r.pos_sim - r.pos_true;
        Vector3d err_est = r.pos_est - r.pos_true;

        bool is_high = isHighPrecision(r.node_id);
        string node_type = is_high ? "High" : "Low";

        // 分发到不同的统计组
        if (is_high) {
            time_stats_high[r.t_ref].add(err_sim, err_est);
            total_stats_high.add(err_sim, err_est);
            if(i> STAGE1_SEC * N_NODE)  total_stats_high_2.add(err_sim, err_est);

        }
        else {
            time_stats_low[r.t_ref].add(err_sim, err_est);
            total_stats_low.add(err_sim, err_est);
            if (i > STAGE1_SEC * N_NODE)  total_stats_low_2.add(err_sim, err_est);
        }
        i++;
        //写入详细 CSV
        f_detail << fixed << setprecision(4) << r.t_ref << "," << r.node_id << "," << node_type << ","
            << r.pos_sim.x() << "," << r.pos_sim.y() << "," << r.pos_sim.z() << ","
            << r.pos_est.x() << "," << r.pos_est.y() << "," << r.pos_est.z() << ","
            << r.pos_true.x() << "," << r.pos_true.y() << "," << r.pos_true.z() << ","
            << err_sim.norm() << "," << err_est.norm() << "\n";
    }
    f_detail.close();

    // 3. 输出统计报告到 TXT
    ofstream f_stat("summary_statistics.txt");
    f_stat << "================ 最小二乘异步算法精度评定报告 ================\n";
    f_stat << fixed << setprecision(4);

    // --- 第1部分：全局总体输出 ---
    f_stat << "==============================================================\n";
    f_stat << "【全局总体统计 】\n\n";
    total_stats_high.printReportBlock(f_stat, "全程RMS统计-高精度点");
    total_stats_low.printReportBlock(f_stat, "全程RMS统计-低精度点");
    total_stats_high_2.printReportBlock(f_stat, "平飞段RMS统计-高精度点");
    total_stats_low_2.printReportBlock(f_stat, "平飞段RMS统计-低精度点");

    
    // --- 第2部分：按时间窗口分别输出 ---
    f_stat << "【分历元（时间窗口）统计 】\n\n";
    for (double t : time_windows) {
        f_stat << ">>>>> 历元 t_ref = " << t << " s <<<<<\n";

        // 如果该时间段有对应数据，则输出
        if (time_stats_high.count(t) > 0) {
            time_stats_high[t].printReportBlock(f_stat, "高精度点");
        }
        if (time_stats_low.count(t) > 0) {
            time_stats_low[t].printReportBlock(f_stat, "低精度点");
        }
    }
    


    f_stat.close();

    cout << "\n详细结果已保存至 detailed_results.csv" << endl;
    cout << "统计报告已保存至 summary.txt" << endl;
}