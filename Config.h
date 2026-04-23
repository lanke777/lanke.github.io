#pragma once

#include <set>
#include <string>
using namespace std;
//全局配置参数 (Configuration)

extern double SIGMA_0;           // 基准标准差
extern double SIGMA_RANGE;       // 测距误差
extern double SIGMA_POS_LOW;   // 低精度惯导误差
extern double SIGMA_POS_HIGH;     // 高精度惯导误差
extern int    N_NODE; //总节点数
extern double STAGE1_SEC; //平飞段开始时间,也即直飞段时长 (秒)
extern string FILE_PREFIX; //输入输出文件前缀 X(Y)，其中 X 是节点总数，Y 是高精度点数量 
extern std::set<int> KNOWN_NODE_IDS;

// 辅助函数判断是否为高精度点
inline bool isHighPrecision(int node_id) {
    return KNOWN_NODE_IDS.find(node_id) != KNOWN_NODE_IDS.end();
}