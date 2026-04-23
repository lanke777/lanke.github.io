#include "DataLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

bool DataLoader::loadGatherFile(const string& filename, vector<SimDataRow>& dataset) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[Error] Cannot open " << filename << endl;
        return false;
    }

    string line;
    // 跳过前两行表头
    getline(file, line);
    getline(file, line);

    while (getline(file, line)) {
        if (line.size() < 10) continue;
        SimDataRow row = parseGatherLine(line);
        dataset.push_back(row);
    }
    return true;
}

double DataLoader::parseTimeStr(const string& time_str) {
    if (time_str.empty()) return 0.0;

    string temp = time_str;
    for (char& c : temp) if (c == ':') c = ' '; // 把冒号变空格

    stringstream ss(temp);
    vector<double> parts;
    double val;
    // 动态读取：无论有几个数字都能读进来
    while (ss >> val) {
        parts.push_back(val);
    }

    // 根据读到的部分计算秒数
    if (parts.size() == 3) return parts[0] * 3600.0 + parts[1] * 60.0 + parts[2]; // 时 分 秒
    if (parts.size() == 2) return parts[0] * 60.0 + parts[1];                     // 分 秒
    if (parts.size() == 1) return parts[0];                                       // 只有秒

    return 0.0;
}

int DataLoader::parseIdStr(const string& id_str) {
    string clean_id;
    for (char c : id_str) {
        if (isdigit(c)) clean_id += c;
    }
    return clean_id.empty() ? -1 : stoi(clean_id);
}

SimDataRow DataLoader::parseGatherLine(const string& line) {
    SimDataRow row = {};
    stringstream ss(line);
    string item;

    vector<string> columns;

    // 按逗号分割 CSV 字段
    while (getline(ss, item, ',')) {
        columns.push_back(item);
    }

    // 列数校验，避免无效空行
    if (columns.size() < 10) return row;

    // 安全转换辅助函数，防止空列或下标越界引发异常
    auto toDouble = [&](size_t idx) {
        if (idx >= columns.size() || columns[idx].empty()) return 0.0;
        try { return stod(columns[idx]); }
        catch (...) { return 0.0; }
    };

    // 3. 清理时间戳字符串中的多余格式字符
    string clean_time = columns[0];
    clean_time.erase(remove(clean_time.begin(), clean_time.end(), '\"'), clean_time.end());
    clean_time.erase(remove(clean_time.begin(), clean_time.end(), '\t'), clean_time.end());
    clean_time.erase(remove(clean_time.begin(), clean_time.end(), ' '), clean_time.end());

    row.time_str = clean_time;
    row.timestamp = parseTimeStr(clean_time);

    row.m_id = parseIdStr(columns[1]);
    row.s_id = parseIdStr(columns[2]);

    // 4. 读取主节点观测值 (Idx 3-14)
    row.m_sim.vx = toDouble(3); row.m_sim.vy = toDouble(4); row.m_sim.vz = toDouble(5);
    row.m_sim.ax = toDouble(6); row.m_sim.ay = toDouble(7); row.m_sim.az = toDouble(8);
    row.m_sim.pitch = toDouble(9); row.m_sim.roll = toDouble(10); row.m_sim.yaw = toDouble(11);
    row.m_sim.X = toDouble(12); row.m_sim.Y = toDouble(13); row.m_sim.Z = toDouble(14);

    // 5. 读取从节点观测值 (Idx 15-26)
    row.s_sim.vx = toDouble(15); row.s_sim.vy = toDouble(16); row.s_sim.vz = toDouble(17);
    row.s_sim.ax = toDouble(18); row.s_sim.ay = toDouble(19); row.s_sim.az = toDouble(20);
    row.s_sim.pitch = toDouble(21); row.s_sim.roll = toDouble(22); row.s_sim.yaw = toDouble(23);
    row.s_sim.X = toDouble(24);  row.s_sim.Y = toDouble(25);  row.s_sim.Z = toDouble(26);

    // 6. 读取测距观测值 (Idx 27)
    row.range_sim = toDouble(27);

    // 7. 读取真值数据 (用于后续精度评定)
    row.m_true.X = toDouble(37); row.m_true.Y = toDouble(38); row.m_true.Z = toDouble(39);
    row.s_true.X = toDouble(49); row.s_true.Y = toDouble(50); row.s_true.Z = toDouble(51);

    return row;
}