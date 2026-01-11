#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <filesystem>

// Undefine Windows macros that conflict with our enum
#ifdef ERROR
#undef ERROR
#endif

namespace sp {

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERR,
    CRITICAL
};

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void set_level(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        min_level_ = level;
    }

    LogLevel get_level() const {
        return min_level_;
    }
    
    void enable_console(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);
        console_enabled_ = enable;
    }
    
    bool enable_file(const std::string& path, size_t max_size_mb = 10) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        log_file_path_ = path;
        max_file_size_ = max_size_mb * 1024 * 1024; // Convert to bytes
        
        log_file_.open(path, std::ios::app);
        if (!log_file_.is_open()) {
            return false;
        }
        
        file_enabled_ = true;
        return true;
    }
    
    void disable_file() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (log_file_.is_open()) {
            log_file_.close();
        }
        file_enabled_ = false;
    }

    template<typename... Args>
    void log(LogLevel level, const char* plugin, Args&&... args) {
        if (level < min_level_) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::ostringstream oss;
        oss << "[" << get_timestamp() << "] "
            << "[" << level_to_string(level) << "] ";
        
        if (plugin && plugin[0] != '\0') {
            oss << "[" << plugin << "] ";
        }
        
        (oss << ... << args);
        
        std::string message = oss.str();
        
        // Write to console
        if (console_enabled_) {
            std::cout << message << std::endl;
        }
        
        // Write to file
        if (file_enabled_ && log_file_.is_open()) {
            log_file_ << message << std::endl;
            log_file_.flush();
            
            // Check file size and rotate if needed
            check_and_rotate_log();
        }
    }

    template<typename... Args>
    void trace(const char* plugin, Args&&... args) {
        log(LogLevel::TRACE, plugin, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(const char* plugin, Args&&... args) {
        log(LogLevel::DEBUG, plugin, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(const char* plugin, Args&&... args) {
        log(LogLevel::INFO, plugin, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(const char* plugin, Args&&... args) {
        log(LogLevel::WARN, plugin, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(const char* plugin, Args&&... args) {
        log(LogLevel::ERR, plugin, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(const char* plugin, Args&&... args) {
        log(LogLevel::CRITICAL, plugin, std::forward<Args>(args)...);
    }

private:
    Logger() : min_level_(LogLevel::INFO), console_enabled_(true), file_enabled_(false), max_file_size_(10 * 1024 * 1024) {}
    
    ~Logger() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static const char* level_to_string(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE:    return "TRACE";
            case LogLevel::DEBUG:    return "DEBUG";
            case LogLevel::INFO:     return "INFO ";
            case LogLevel::WARN:     return "WARN ";
            case LogLevel::ERR:      return "ERROR";
            case LogLevel::CRITICAL: return "CRIT ";
            default:                 return "?????";
        }
    }

    static std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::ostringstream oss;
        std::tm tm_buf;
        #ifdef _WIN32
        localtime_s(&tm_buf, &time_t);
        #else
        localtime_r(&time_t, &tm_buf);
        #endif
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }
    
    void check_and_rotate_log() {
        if (!log_file_.is_open()) return;
        
        auto current_pos = log_file_.tellp();
        if (current_pos < 0 || static_cast<size_t>(current_pos) < max_file_size_) {
            return;
        }
        
        // Close current file
        log_file_.close();
        
        // Rotate: rename current file
        std::filesystem::path old_path = log_file_path_;
        std::filesystem::path new_path = log_file_path_ + ".1";
        
        std::error_code ec;
        if (std::filesystem::exists(new_path)) {
            std::filesystem::remove(new_path, ec);
        }
        std::filesystem::rename(old_path, new_path, ec);
        
        // Reopen new file
        log_file_.open(log_file_path_, std::ios::app);
    }

    LogLevel min_level_;
    bool console_enabled_;
    bool file_enabled_;
    std::ofstream log_file_;
    std::string log_file_path_;
    size_t max_file_size_;
    std::mutex mutex_;
};

#define SP_TRACE(...) ::sp::Logger::instance().trace("", __VA_ARGS__)
#define SP_DEBUG(...) ::sp::Logger::instance().debug("", __VA_ARGS__)
#define SP_INFO(...)  ::sp::Logger::instance().info("", __VA_ARGS__)
#define SP_WARN(...)  ::sp::Logger::instance().warn("", __VA_ARGS__)
#define SP_ERROR(...) ::sp::Logger::instance().error("", __VA_ARGS__)
#define SP_CRIT(...)  ::sp::Logger::instance().critical("", __VA_ARGS__)

#define PLUGIN_TRACE(plugin, ...) ::sp::Logger::instance().trace(plugin, __VA_ARGS__)
#define PLUGIN_DEBUG(plugin, ...) ::sp::Logger::instance().debug(plugin, __VA_ARGS__)
#define PLUGIN_INFO(plugin, ...)  ::sp::Logger::instance().info(plugin, __VA_ARGS__)
#define PLUGIN_WARN(plugin, ...)  ::sp::Logger::instance().warn(plugin, __VA_ARGS__)
#define PLUGIN_ERROR(plugin, ...) ::sp::Logger::instance().error(plugin, __VA_ARGS__)
#define PLUGIN_CRIT(plugin, ...)  ::sp::Logger::instance().critical(plugin, __VA_ARGS__)

} // namespace sp
