#include "Logger.h"
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <filesystem>

using namespace Zero::Logger;
using namespace std;
namespace fs = std::filesystem;  // 命名空间别名

const char* Logger::s_level[LEVEL_COUNT] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

Zero::Logger::Logger* Zero::Logger::Logger::m_instance = nullptr;

Logger::Logger() :m_level(DEBUG), m_max(0), m_len(0) {
    m_writeThread = make_unique<thread>(&Logger::writeThreadFunc, this);
}

Logger::~Logger() {

    // 设置关闭标志
    m_shutdown = true;

    // 唤醒写入线程
    m_bufferReady.notify_one();

    // 等待线程结束
    if (m_writeThread && m_writeThread->joinable()) {
        m_writeThread->join();
    }

    close();
}

void Logger::flushRemainingLogs() {
    lock_guard<mutex> bufferLock(m_bufferMutex);
    lock_guard<mutex> fileLock(m_mutex);

    if (!m_frontBuffer.empty()) {}
    {
        for (const auto& msg : m_frontBuffer)
        {
            m_fout << msg << "\n";
        }
        m_frontBuffer.clear();
    }
}

Logger* Logger::instance() {
    if (m_instance == nullptr)
    {
        m_instance = new Logger();
    }
    return m_instance;
}

void Logger::open(const string& filename) {
    m_filename = filename;
    m_fout.open(filename, ios::app);
    if (m_fout.fail())
    {
        throw logic_error("open file faild" + filename);
    }
    m_fout.seekp(0, ios::end);
    m_len = m_fout.tellp();
}

void Logger::close() {
    m_fout.close();
}

void Logger::log(Level level, const char* file, int line, const char* format, ...) {
    if (m_level > level)
    {
        return;
    }

    // 1. 格式化日志消息 (无锁操作)
    va_list args;
    va_start(args, format);
    char buffer[MAX_LOG_LENGTH];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // 2. 构造完整日志条目
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);

    stringstream ss;
    ss << put_time(localtime(&now_c), "%F %T") << " "
       << s_level[level] << " "
       << file << ":" << line << " "
       << buffer;

    {
        lock_guard<mutex> lock(m_bufferMutex);
        m_frontBuffer.emplace_back(ss.str());
    }

    // 4. 通知消费者线程
    m_bufferReady.notify_one();
}

void Logger::writeThreadFunc() {
    const size_t MAX_BATCH_SIZE = 100; // 最大批量写入条数
    while (true)
    {
        // 1. 加锁检查缓冲区和关闭标志
        {
            unique_lock<mutex> lock(m_bufferMutex);
            
             // 等待缓冲区有数据或关闭信号
            m_bufferReady.wait(lock, [this]() {
                return !m_frontBuffer.empty() || m_shutdown;
            });
            
            if (m_shutdown && m_frontBuffer.empty())
            {
                break;
            }

            //交换缓冲区
            // swap(m_frontBuffer, m_backBuffer);
            m_backBuffer = move(m_frontBuffer);
            m_frontBuffer.clear();
        }
        
        // 2. 无锁批量写入文件
        if (!m_backBuffer.empty())
        {
            // m_fout
            try
            {
                lock_guard<mutex> lock(m_mutex);
                
                int written = 0;
                while (written < m_backBuffer.size()) 
                {
                    int end = min(written + MAX_BATCH_SIZE, m_backBuffer.size());

                    for (int i = written; i < end; i++)
                    {
                        m_fout << m_backBuffer[i] << "\n";
                    }
                    m_fout.flush();
                    written = end;
                }

                // 更新日志长度
                // m_len = m_fout.tellp();
                streampos pos = m_fout.tellp();
                if (pos != -1) {
                    m_len = pos;
                } else {
                    m_len = fs::file_size(m_filename);
                }
                // 检查日志轮转
                if (m_max > 0 && m_len >= m_max)
                {
                    rotate();
                }
            } catch(...)  {
                std::cerr << "Log write failed" << '\n';
            }
        }

        // 3. 清空后台缓冲区
        m_backBuffer.clear();
    }
    
    // 线程退出前确保所有日志写入
    flushRemainingLogs();
}

void Logger::rotate() {
    // lock_guard<mutex> lock(m_mutex);
    close();

    time_t ticks = time(nullptr);
    struct tm* ptm = localtime(&ticks);
    char timestamp[32];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), ".%Y-%m-%d_%H-%M-%S", ptm);

    string filename = m_filename + timestamp;
    if (rename(m_filename.c_str(), filename.c_str()) != 0)
    {
        throw logic_error("rename log file failed: " + string(strerror(errno)));
    }
    open(m_filename);
}