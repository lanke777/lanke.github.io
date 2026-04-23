#pragma once
#include "DataTypes.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

/// @brief 数据加载器，负责从 CSV 文件中读取和解析观测数据
class DataLoader {
public:
    /// @brief 从指定路径加载采集数据
    /// @param filename CSV 文件路径
    /// @param dataset 用于存放解析结果的容器
    /// @return 读取是否成功
    static bool loadGatherFile(const string& filename, vector<SimDataRow>& dataset);


private:
    //工具1: 解析时间字符串 "HH:MM:SS.mmm"->总秒数 double
    static double parseTimeStr(const string& time_str); 
    //工具2: 解析 ID "id1" -> 1
    static int parseIdStr(const string& id_str);
    //工具2: 解析 CSV 的辅助行函数
    static SimDataRow parseGatherLine(const string& line);
};