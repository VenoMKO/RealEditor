#pragma once
#include <mutex>
#include <string>
#include <vector>
#include <ctime>

struct ALogEntry {
  enum class Type
  {
    NONE = 0,
    ERR,
    WARN,
    INFO
  };
  std::time_t Time = 0;
  std::string Text;
  Type Channel = Type::NONE;
};

class ALog {
public:
  static ALog* SharedLog();

  static void ILog(const std::string& msg);
  static void ELog(const std::string& msg);
  static void WLog(const std::string& msg);
  
  void GetEntries(std::vector<ALogEntry>& output, size_t& index);

private:
  static void Log(const std::string& msg, ALogEntry::Type channel);
  void Push(const ALogEntry& entry);

private:
  std::recursive_mutex EntriesMutex;
  std::vector<ALogEntry> Entries;
};