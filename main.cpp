#include <iostream>
#include <string>
#include <cstdio>
#include <sstream> // 引入 sstream 处理单行多个输入
#include <vector>
#include "nlohmann/json.hpp"
#include "Config.h"
#include "DataLoader.h"
#include "AdjustmentEngine.h"

// 声明解析 JSON 的函数
bool loadConfigFromJson(const std::string& filepath, AppConfig& config);

int main() {
    std::cout << "=========================================" << std::endl;
    std::cout << "      欢迎使用异步协同定位平差软件       " << std::endl;
    std::cout << "=========================================" << std::endl;

    while (true) {
        std::cout << "\n请输入要批处理的节点参数序列 (用空格分隔) [或输入 q 退出]:\n> ";

        std::string inputLine;
        // 获取整行输入
        std::getline(std::cin, inputLine);

        // 处理直接回车的情况
        if (inputLine.empty()) continue;

        // 检查是否退出
        if (inputLine == "q" || inputLine == "Q") {
            std::cout << "程序退出。再见！" << std::endl;
            break;
        }

        // 使用 stringstream 按空格分割这一行的输入
        std::stringstream ss(inputLine);
        std::string param;
        std::vector<std::string> taskQueue;

        while (ss >> param) {
            taskQueue.push_back(param);
        }

        std::cout << ">>> 成功解析到 " << taskQueue.size() << " 个批处理任务！开始按顺序执行...\n" << std::endl;

        // 遍历执行每一个任务
        for (size_t i = 0; i < taskQueue.size(); ++i) {
            const std::string& currentTask = taskQueue[i];

            std::cout << "-----------------------------------------" << std::endl;
            std::cout << "[任务 " << (i + 1) << "/" << taskQueue.size() << "] 正在处理: " << currentTask << std::endl;

            int totalNodes = 0;
            int highPrecisionNodes = 0;

            // 验证格式
            if (std::sscanf(currentTask.c_str(), "%d(%d)", &totalNodes, &highPrecisionNodes) != 2) {
                std::cerr << ">>> 错误：任务格式不正确 (" << currentTask << ")，跳过当前任务。" << std::endl;
                continue;
            }

            // 拼接文件名
            std::string configFileName = currentTask + "_cfg.json";
            std::string dataFileName = currentTask + ".csv";

            std::cout << ">>> 加载配置: " << configFileName << std::endl;
            std::cout << ">>> 加载数据: " << dataFileName << std::endl;

            AppConfig config;
            config.totalNodes = totalNodes;
            config.highPrecisionNodes = highPrecisionNodes;
            config.configFilePath = dataFileName; // 把数据文件名传进去给 DataLoader

            // 1. 加载 JSON
            if (!loadConfigFromJson(configFileName, config)) {
                std::cerr << ">>> 错误：跳过当前任务 " << currentTask << std::endl;
                continue;
            }

            // -----------------------------------------------------------------
            // 核心业务流
            // -----------------------------------------------------------------
            DataLoader loader;
            // if (!loader.loadData(config)) {
            //     std::cerr << ">>> 错误：无法加载数据文件 " << dataFileName << std::endl;
            //     continue;
            // }

            AdjustmentEngine engine;
            // engine.initialize(config);

            // bool success = engine.solve();
            bool success = true; // 占位，测试用

            if (success) {
                std::cout << ">>> [成功] " << currentTask << " 平差计算完成！" << std::endl;
            }
            else {
                std::cerr << ">>> [失败] " << currentTask << " 平差未收敛。" << std::endl;
            }
        }
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << ">>> 批处理队列执行完毕！" << std::endl;
    }

    return 0;
}