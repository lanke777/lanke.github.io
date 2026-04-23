#pragma once
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

//核心数据结构 (Data Structures) 

//用于存储单节点的状态数据
struct Node {
    int id;
    int matrix_idx;
    //核心状态量
    Vector3d pos_est;       // t_ref 时刻的坐标估计值

    // 速度变化曲线，存储历元内的所有速度切片 <时间戳, 速度向量>
    map<double, Vector3d> velocity_curve;

    Node(int _id = 0) : id(_id), matrix_idx(-1) {
        pos_est.setZero();
    }

    // 状态归算：基于速度曲线的积分计算
    Vector3d predictPos(double tk, double t_ref) const;
};

// 用于保存统计结果的结构体
struct ResultRecord {
    double t_ref;       // 归算时刻
    int node_id;        // 节点ID
    Vector3d pos_sim;   // 平差前(初始)坐标
    Vector3d pos_est;   // 平差后坐标
    Vector3d pos_true;  // 真值坐标
};

// 线性化后的观测因子，把算出的导数和残差，打包送给平差函数求解
struct ObsFactor {
    bool is_valid;          // 数据是否有效 (例如距离太近会导致无效)
    bool is_ranging;        // true=测距, false=INS位置

    // --- 参与的节点索引 ---
    int idx_i;              // 主节点在矩阵中的索引 (Master)
    int idx_j;              // 从节点在矩阵中的索引 (Slave, INS模式下为-1)

    // --- 核心数学量 ---
    double weight;          // 权值 p (测距为标量，INS取均值或单独处理)
    Matrix3d weight_mat;    // 权矩阵 P (仅INS使用)

    double residual;        // 测距残差 l = Obs - Comp
    Vector3d residual_vec;  // INS残差向量 l_vec

    // --- 雅可比核心 (方向余弦) ---
    // 对应文档中的向量 [a, b, c]^T
    // 测距法方程核心矩阵 M = u * u.transpose()
    Vector3d u_vec;
};


// 观测类型
enum ObsType {
    OBS_RANGING, // 测距观测 (标量距离)
    OBS_INS      // 惯导位置观测 (3D坐标)
};

// 观测值 (Observation)
struct Observation {
    ObsType type;       // 观测类型
    double timestamp;   // 观测发生的具体时刻 tk

    int master_id;      // 主节点 ID (INS主体 或 测距发起方)
    int slave_id;       // 从节点 ID (测距响应方，INS模式下为-1)

    // 观测数值
    // 测距模式: value.x() = 距离测量值
    // INS模式:  value = (x, y, z) 坐标测量值
    Vector3d value;

    // 速度信息
    // 携带该观测时刻的速度向量，用于更新节点的 Velocity 属性
    Vector3d velocity;

    // 测距: sigma.x() = 测距标准差
    // INS:  sigma = (sx, sy, sz) 位置标准差
    Vector3d sigma;

    // [函数] 获取权矩阵 (Weight Matrix P)
    // 根据定权公式: P = (sigma_0 / sigma_obs)^2
    Matrix3d getWeightMatrix(double unit_sigma) const;

    // 默认构造
    Observation() : type(OBS_INS), timestamp(0), master_id(-1), slave_id(-1) {
        value.setZero(); velocity.setZero(); sigma.setOnes();
    }
    // 构造函数 1: 测距
    Observation(double t, int master, int slave, double range, const Vector3d& vel, double s_dist)
        : type(OBS_RANGING), timestamp(t), master_id(master), slave_id(slave) {
        value = Vector3d(range, 0, 0);
        velocity = vel;
        sigma = Vector3d(s_dist, 1.0, 1.0);
    }
    // 构造函数 2: INS
    Observation(double t, int master, const Vector3d& pos, const Vector3d& vel, const Vector3d& s_pos)
        : type(OBS_INS), timestamp(t), master_id(master), slave_id(-1) {
        value = pos;
        velocity = vel;
        sigma = s_pos;
    }

};

//CSV 原始数据结构

// 1. 节点状态 
struct NodeState {
    double vx, vy, vz;
    double ax, ay, az;
    double pitch, roll, yaw;
    double X, Y, Z;
};

// 2. CSV 行数据 (对应 gather.csv 的每一行)
struct SimDataRow {
    string time_str;      // 保存原始时间字符串
    double timestamp;     // 转换后的秒数
    int m_id;            // 主节点 ID (数字)
    int s_id;            // 从节点 ID (数字)

    // 观测值
    NodeState m_sim;
    NodeState s_sim;
    double range_sim;

    // 真值 (用于最后做精度对比)
    NodeState m_true;
    NodeState s_true;
    double range_true;
};