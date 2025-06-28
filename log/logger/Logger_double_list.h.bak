#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <vector>

using namespace std;

namespace Zero {
namespace Logger {

#define debug(format, ...) \
    Logger::instance()->log(Logger::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__);

#define info(format, ...) \
    Logger::instance()->log(Logger::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__);

#define warn(format, ...) \
    Logger::instance()->log(Logger::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__);

#define error(format, ...) \
    Logger::instance()->log(Logger::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__);

#define fatal(format, ...) \
    Logger::instance()->log(Logger::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__);

class Logger
{
public:
    enum Level
    {
        DEBUG = 0,
        INFO,
        WARN,
        ERROR,
        FATAL,
        LEVEL_COUNT
    };
    static Logger* instance();
    void open(const string& filename);
    void close();
    void log(Level level, const char* file, int line, const char* format, ...);
    void writeThreadFunc();
    void flushRemainingLogs();
    
    void level(Level level) {
        m_level = level;
    }

    void max(int bytes) {
        m_max = bytes;
    }
private:
    Logger(/* args */);
    ~Logger();
    void rotate();
private:
    static constexpr int MAX_BUFFER_SIZE = 10000;

    static constexpr int MAX_LOG_LENGTH = 1024;

    string m_filename;
    ofstream m_fout;
    Level m_level;
    mutex m_mutex;

    // 双缓冲区
    vector<std::string> m_frontBuffer;  // 前台缓冲(生产者写入)
    vector<std::string> m_backBuffer;   // 后台缓冲(消费者写入文件)

    // 同步控制
    mutex m_bufferMutex;                // 保护缓冲区交换
    condition_variable m_bufferReady;   // 缓冲区就绪信号
    atomic<bool> m_shutdown{false};     // 关闭标志

    // 写入线程
    unique_ptr<std::thread> m_writeThread;

    int m_max;
    int m_len;
    static const char* s_level[LEVEL_COUNT];
    static Logger* m_instance;
};    
} // namespace Logger
} // namespace Zero
