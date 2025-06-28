#include "Logger.h"
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>

using namespace std;
using namespace Zero::Logger;

// 测试基本功能
void testBasicFunctionality() {
    cout << "=== 测试基本功能 ===" << endl;
    
    Logger* logger = Logger::instance();
    logger->open("test_basic.log");
    logger->level(Logger::DEBUG); // 设置最低日志级别
    
    debug("这是一条DEBUG日志");
    info("这是一条INFO日志");
    warn("这是一条WARN日志");
    error("这是一条ERROR日志");
    fatal("这是一条FATAL日志");
    
    // 测试日志级别过滤
    logger->level(Logger::WARN);
    info("这条INFO日志不应该出现.");
    warn("这条WARN日志应该出现");
    
    cout << "基本功能测试完成，请检查test_basic.log文件" << endl;
}

// 测试日志轮转
void testLogRotation() {
    cout << "=== 测试日志轮转 ===" << endl;
    
    Logger* logger = Logger::instance();
    logger->open("test_rotation.log");
    logger->max(1024); // 设置1KB后轮转
    logger->level(Logger::DEBUG);
    
    // 写入足够多的日志触发轮转
    for (int i = 0; i < 1000; ++i) {
        info("这是第%d条测试日志，用于测试日志轮转功能，确保日志文件达到指定大小后会自动轮转.", i);
    }
    
    // 确保所有日志写入文件
    logger->flushRemainingLogs();
    
    // 等待写入线程完成
    this_thread::sleep_for(chrono::milliseconds(100));
    
    cout << "日志轮转测试完成，检查test_rotation.log和它的备份文件." << endl;
}

// 单线程性能测试
void testSingleThreadPerformance() {
    cout << "=== 单线程性能测试 ===" << endl;
    
    Logger* logger = Logger::instance();
    logger->open("test_performance.log");
    logger->level(Logger::INFO);
    
    const int TEST_COUNT = 100000;
    auto start = chrono::high_resolution_clock::now();
    
    for (int i = 0; i < TEST_COUNT; ++i) {
        info("性能测试日志 %d - 这是一条用于测试日志系统性能的日志消息.", i);
    }
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << "写入 " << TEST_COUNT << " 条日志耗时: " << duration << " 毫秒." << endl;
    cout << "平均吞吐量: " << (TEST_COUNT * 1000 / duration) << " 条/秒." << endl;
}

// 多线程测试函数
void threadFunc(int threadId, int logCount) {
    Logger* logger = Logger::instance();
    
    for (int i = 0; i < logCount; ++i) {
        info("线程%d - 日志%d", threadId, i);
        
        // 随机休眠0-10ms模拟实际工作负载
        this_thread::sleep_for(chrono::milliseconds(rand() % 10));
    }
}

// 多线程并发测试
void testMultiThreading() {
    cout << "=== 多线程并发测试 ===" << endl;
    
    Logger* logger = Logger::instance();
    logger->open("test_multithread.log");
    logger->level(Logger::INFO);
    
    const int THREAD_COUNT = 10;
    const int LOGS_PER_THREAD = 1000;
    
    vector<thread> threads;
    auto start = chrono::high_resolution_clock::now();
    
    // 创建并启动多个线程
    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(threadFunc, i, LOGS_PER_THREAD);
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << THREAD_COUNT << " 个线程，每个写入 " << LOGS_PER_THREAD << " 条日志." << endl;
    cout << "总日志量: " << (THREAD_COUNT * LOGS_PER_THREAD) << " 条." << endl;
    cout << "总耗时: " << duration << " 毫秒" << endl;
    cout << "平均吞吐量: " << (THREAD_COUNT * LOGS_PER_THREAD * 1000 / duration) << " 条/秒." << endl;
}

// 测试缓冲区溢出处理
void testBufferOverflow() {
    cout << "=== 测试缓冲区溢出处理 ===" << endl;
    
    Logger* logger = Logger::instance();
    logger->open("test_overflow.log");
    logger->level(Logger::INFO);
    
    // 快速写入大量日志，测试缓冲区处理
    const int TEST_COUNT = 100000;
    auto start = chrono::high_resolution_clock::now();
    
    for (int i = 0; i < TEST_COUNT; ++i) {
        info("缓冲区溢出测试日志 %d - 这是一条用于测试日志系统缓冲区处理能力的日志消息.", i);
    }
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << "写入 " << TEST_COUNT << " 条日志耗时: " << duration << " 毫秒" << endl;
    cout << "检查test_overflow.log文件是否完整" << endl;
}

int main() {
    try {
        // 运行各种测试
        testBasicFunctionality();
        testLogRotation();
        testSingleThreadPerformance();
        testMultiThreading();
        // testBufferOverflow();//暂未实现。
        
        cout << "所有测试完成!" << endl;
    } catch (const exception& e) {
        cerr << "测试失败: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}