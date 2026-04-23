#pragma once

#include <vector>
#include <map>
#include "DataTypes.h"  
#include "DataLoader.h" 

class Helper {
public:

 ///@brief 预处理指定时间间隔内的原始数据行，并构建标准测距和惯导观测值
 ///@param interval_data 当前时间间隔内的原始测量数据集合
 ///@param global_nodes  全局节点字典。用于存储节点状态和历史速度切片
 ///@param obs_list      输出的标准化观测值列表（含 INS 与测距观测），作为平差输入
 static void prepareIntervalData(
        const std::vector<SimDataRow>& interval_data,
        std::map<int, Node>& global_nodes,
        std::vector<Observation>& obs_list
    );
};