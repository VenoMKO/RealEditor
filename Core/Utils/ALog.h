#pragma once
#include <mutex>
#include <string>
#include <vector>
#include <ctime>
#include <Tera/CoreStrings.h>

struct ALogEntry {
  enum class Type
  {
    NONE = 0,
    ERR,
    WARN,
    INFO,
    USR,
    PERF,
    DBG
  };
  std::time_t Time = 0;
  std::string Text;
  Type Channel = Type::NONE;
};

class ALog {
public:
  static ALog* SharedLog();
  static void Log(const std::string& msg, ALogEntry::Type channel);
  
  void GetEntries(std::vector<ALogEntry>& output, size_t& index);

private:
  
  void Push(const ALogEntry& entry);

private:
  std::recursive_mutex EntriesMutex;
  std::vector<ALogEntry> Entries;
};

#define LogI(...) ALog::Log(Sprintf(__VA_ARGS__), ALogEntry::Type::INFO)
#define LogW(...) ALog::Log(Sprintf(__VA_ARGS__), ALogEntry::Type::WARN)
#define LogE(...) ALog::Log(Sprintf(__VA_ARGS__), ALogEntry::Type::ERR)
#define LogU(...) ALog::Log(Sprintf(__VA_ARGS__), ALogEntry::Type::USR)
#define LogD(...) ALog::Log(Sprintf(__VA_ARGS__), ALogEntry::Type::DBG)
#define LogP(...) ALog::Log(Sprintf(__VA_ARGS__), ALogEntry::Type::PERF)