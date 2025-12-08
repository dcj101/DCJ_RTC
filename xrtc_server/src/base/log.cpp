
#include <iostream>

#include <sys/stat.h>

#include "base/log.h"

namespace xrtc {

XrtcLog::XrtcLog(const std::string& log_dir,
        const std::string& log_name,
        const std::string& log_level) :
    _log_dir(log_dir),
    _log_name(log_name),
    _log_level(log_level),
    _log_file(log_dir + "/" + log_name + ".log"),
    _log_file_wf(log_dir + "/" + log_name + ".log.wf")
{

}

XrtcLog::~XrtcLog() {
    stop();

    _out_file.close();
    _out_file_wf.close();
}

void XrtcLog::OnLogMessage(const std::string& message, 
        rtc::LoggingSeverity severity)
{
    // 这里可以在调用rtc底层日志的时候 LogSink会把日志回调回来，支持用户自定义行为 比如把日志写到本地去
    if (severity >= rtc::LS_WARNING) {
        std::unique_lock<std::mutex> lock(_mtx_wf);
        _log_queue_wf.push(message);
    } else {
        std::unique_lock<std::mutex> lock(_mtx);
        _log_queue.push(message);
    }
}

void XrtcLog::OnLogMessage(const std::string& /*message*/) {
    // 不需要有逻辑
}

static rtc::LoggingSeverity get_log_severity(const std::string& level) {
    if ("verbose" == level) {
        return rtc::LS_VERBOSE;
    } else if ("info" == level) {
        return rtc::LS_INFO;
    } else if ("warning" == level) {
        return rtc::LS_WARNING;
    } else if ("error" == level) {
        return rtc::LS_ERROR;
    } else if ("none" == level) {
        return rtc::LS_NONE;
    }
    
    return rtc::LS_NONE;
}

int XrtcLog::init() {
    // 把log模块添加到rtc的log sink中
    // 输出线程信息 + 系统相对的启动时间
    rtc::LogMessage::ConfigureLogging("thread tstamp");
    // 设置log文件的路径前缀 匹配到src的路径就停止
    rtc::LogMessage::SetLogPathPrefix("/src");
    // 设置log的severity等级
    rtc::LogMessage::AddLogToStream(this, get_log_severity(_log_level));

    int ret = mkdir(_log_dir.c_str(), 0755);
    // 如果不是目录不存在
    if (ret != 0 && errno != EEXIST) {
        fprintf(stderr, "create log_dir[%s] failed\n", _log_dir.c_str());
        return -1;
    }
    
    // 打开文件,按照文件追加模式
    _out_file.open(_log_file, std::ios::app);
    if (!_out_file.is_open()) {
        fprintf(stderr, "open log_file[%s] failed\n", _log_file.c_str());
        return -1;
    }
    
    _out_file_wf.open(_log_file_wf, std::ios::app);
    if (!_out_file_wf.is_open()) {
        fprintf(stderr, "open log_file_wf[%s] failed\n", _log_file_wf.c_str());
        return -1;
    }

    return 0;
}

bool XrtcLog::start() {
    if (_running) {
        fprintf(stderr, "log thread already running\n");
        return false;
    }

    _running = true;

    _thread = new std::thread([=]() {
        struct stat stat_data;
        std::stringstream ss;

        while (_running) {
            // 检查日志文件是否被删除或者移动
            if (stat(_log_file.c_str(), &stat_data) < 0) {
                _out_file.close();
                _out_file.open(_log_file, std::ios::app);
            }
            
            if (stat(_log_file_wf.c_str(), &stat_data) < 0) {
                _out_file_wf.close();
                _out_file_wf.open(_log_file_wf, std::ios::app);
            }
           

            bool write_log = false;
            {
                std::unique_lock<std::mutex> lock(_mtx);
                if (!_log_queue.empty()) {
                    write_log = true;
                    while (!_log_queue.empty()) {
                        ss << _log_queue.front();
                        _log_queue.pop();
                    }
                }
            }
            // 为了降低锁的粒度 先判断是否有日志需要写入
            if (write_log) {
                _out_file << ss.str();
                _out_file.flush();
            }
            
            ss.str("");

            bool write_log_wf = false;
            {
                std::unique_lock<std::mutex> lock(_mtx_wf);
                if (!_log_queue_wf.empty()) {
                    write_log_wf = true;
                    while (!_log_queue_wf.empty()) {
                        ss << _log_queue_wf.front();
                        _log_queue_wf.pop();
                    }
                }
            }

            if (write_log_wf) {
                _out_file_wf << ss.str();
                _out_file_wf.flush();
            }
          
            ss.str("");
            // 每30ms检查一次日志队列
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        
    });
    
    return true;
}

void XrtcLog::stop() {
    _running = false;

    if (_thread) {
        if (_thread->joinable()) {
            _thread->join();
        }

        delete _thread;
        _thread = nullptr;
    }
}

void XrtcLog::join() {
    if (_thread && _thread->joinable()) {
        _thread->join();
    }
}

void XrtcLog::set_log_to_stderr(bool on) {
    rtc::LogMessage::SetLogToStderr(on);
}

} // namespace xrtc


