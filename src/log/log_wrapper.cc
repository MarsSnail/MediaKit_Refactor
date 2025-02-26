#include "log/log_wrapper.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include <windows.h>

#include "boost/assert.hpp"
#include <string>
#include <codecvt>
#include <mutex>
#include <boost/filesystem.hpp>
#include "libavutil/log.h"
#include "base/util.h"

const char* kLogFileName = "mediakit.log";
const int kMaxLogFileSize = 1024 * 1024 * 5;  // 5M
const int kMaxLogFileCount = 5;
const char* kLoggerName = "snail_messenger";

namespace media {
std::mutex log_init_mutex;

void ffmpeg_log_callback(void* avcl, int level, const char* fmt, va_list vl) {
  std::string log_content = base::string_format(fmt, vl);
  LogLevel loglevel = LOG_LEVEL_DEBUG;
  switch(level) {
  case AV_LOG_ERROR:
    loglevel = LOG_LEVEL_ERROR;
    break;
  case AV_LOG_DEBUG:
    loglevel = LOG_LEVEL_DEBUG;
    break;
  case AV_LOG_INFO:
    loglevel = LOG_LEVEL_INFO;
    break;
  default:
    loglevel = LOG_LEVEL_INFO;
  }
  LOGGING(loglevel)<<log_content;
}

std::wstring GetApplicationFileDirUtf16() {
  TCHAR szPath[MAX_PATH];
  ::GetModuleFileName(NULL, szPath, MAX_PATH);
  std::wstring path(szPath);
  std::wstring file_name;
  if (path.find_last_of(L'/') != std::wstring::npos) {
    file_name = path.substr(0, path.find_last_of(L'/'));
  } else if (path.find_last_of(L'\\') != std::wstring::npos) {
    file_name = path.substr(0, path.find_last_of(L'\\'));
  }
  return file_name;
}

std::string GetApplicationFileDirUtf8() {
  std::wstring file_dir_utf16 = GetApplicationFileDirUtf16();
  return UTF16toUTF8(file_dir_utf16);
}

std::string UTF16toUTF8(const std::wstring& str) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
  std::string utf8_string = convert.to_bytes(str);
  return utf8_string;
}

void InitializeLog(){
  std::unique_lock<std::mutex> lck(log_init_mutex);
  static bool initialized = false;
  if(initialized) {
    return;
  }
  spdlog::drop_all();
  std::string log_file_dir = UTF16toANSI(GetApplicationFileDirUtf16()) + "/log/";
  if(!boost::filesystem::is_directory(log_file_dir)) {
    bool result =  boost::filesystem::create_directory(log_file_dir);
    if(!result) {
      BOOST_ASSERT_MSG(0, "Create directory fiale");
      return;
    }
  }
  std::string log_file_path = log_file_dir + kLogFileName;
  auto message_logger = spdlog::rotating_logger_mt<spdlog::async_factory>(
    kLoggerName,
    log_file_path,
    kMaxLogFileSize,
    kMaxLogFileCount); 
  message_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
  // 来设置当前的日志等级，所有的等级如下（从大到小）
  // critical(致命错误) err(错误) warn(警告) info(信息) debug(调试) trace(跟踪)
  // 最后显示出来的信息只会是你指定等级及其左边的等级日志
  message_logger->set_level(spdlog::level::trace);
  spdlog::register_logger(message_logger);
  initialized =true;
}

static bool enable_avpacket_process_track = false;

void EnableAVPacketProcessTrace() {
  enable_avpacket_process_track = true;
}

void DisableAVPacketProcessTrace() {
  enable_avpacket_process_track = false;
}

bool IsAVPacketProcessTraceEnabled() {
  return enable_avpacket_process_track;
}

void LogMessage(LogLevel log_level, const std::string& message) {
  std::shared_ptr<spdlog::logger> logger = spdlog::get(kLoggerName);
  if(logger == nullptr) {
    InitializeLog();
    logger = spdlog::get(kLoggerName);
  }
  BOOST_ASSERT_MSG(logger, "log system initialize failed");
  switch(log_level) {
  case LOG_LEVEL_DEBUG:
     logger->debug(message);
    break;
  case LOG_LEVEL_ERROR:
     logger->error(message);
    break;
  case LOG_LEVEL_INFO:
     logger->info(message);
    break;
  case LOG_LEVEL_WRANING:
     logger->warn(message);
    break;
  default:
    BOOST_ASSERT_MSG(0, "unsupported log level");
  }
  logger->flush();
  return;
}

std::string AnsiToUtf8(const std::string& str) {
  int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
  wchar_t* w_str = new wchar_t[len + 1];
  memset(w_str, 0, len + 1);
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, w_str, len);
  len = WideCharToMultiByte(CP_UTF8, 0, w_str, -1, NULL, 0, NULL, NULL);
  char* u_str = new char[len + 1];
  memset(u_str, 0, len + 1);
  WideCharToMultiByte(CP_UTF8, 0, w_str, -1, u_str, len, NULL, NULL);
  std::string ret = u_str;
  if (w_str)
    delete[] w_str;
  if (u_str)
    delete[] u_str;
  return ret;
}

std::string UTF16toANSI(const std::wstring& utf16_str) {
  int len = WideCharToMultiByte(CP_ACP, 0, utf16_str.c_str(), -1, NULL, 0, NULL, NULL);
  char* u_str = new char[len + 1];
  memset(u_str, 0, len + 1);
  WideCharToMultiByte(CP_ACP, 0, utf16_str.c_str(), -1, u_str, len, NULL, NULL);
  std::string ret = u_str;
  if (u_str)
    delete[] u_str;
  return ret;
}

std::string GetMovieNameUtf8(const std::string& movie_file_path) {
    std::string movie_name;
    if (movie_file_path.find_last_of('/') != std::wstring::npos) {
      movie_name = 
        movie_file_path.substr(movie_file_path.find_last_of(L'/') + 1);
    } else if (movie_file_path.find_last_of(L'\\') != std::wstring::npos) {
      movie_name = 
        movie_file_path.substr(movie_file_path.find_last_of('\\') + 1);
    }
    return AnsiToUtf8(movie_name);
  }

FormatLogMessage::FormatLogMessage(const char* file,
  const char* function_name,
  int line, LogLevel log_level): log_level_(log_level) {
    std::string file_name(file);
    auto last_slash_pos = file_name.find_last_of("\\/");
    if (last_slash_pos != std::string::npos) {
      file_name = file_name.substr(last_slash_pos + 1);
    }

    std::string only_function_name(function_name);
    auto last_colon_pos = only_function_name.find_last_of("::");
    if(last_colon_pos != std::string::npos) {
      only_function_name = only_function_name.substr(last_colon_pos + 1);
    }

  format_out_string_stream_<<
    "["<<file_name<<"("<<line<<")"<<"("<< only_function_name <<")"<<"] ";
}

FormatLogMessage::~FormatLogMessage(){
  LogMessage(log_level_, format_out_string_stream_.str());
}

std::ostream& FormatLogMessage::stream() {
  return format_out_string_stream_;
}

} // namespace media