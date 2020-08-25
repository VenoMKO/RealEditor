#pragma once
#include <mutex>
#include <string>
#include <vector>
#include <ctime>

#include "AConfiguration.h"

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

class LogWindow;
class ALog {
public:
  static ALog* SharedLog();

  static void ILog(const std::string& msg);
  static void ELog(const std::string& msg);
  static void WLog(const std::string& msg);

  static void GetConfig(FLogConfig& cfg);
  static void SetConfig(const FLogConfig& cfg);

  static void Show(bool show = true);
  static bool IsShown();

  ALog();

  void OnLogClose();
  void OnAppExit();
  void GetEntries(std::vector<ALogEntry>& output, size_t& index);

private:
  static void Log(const std::string& msg, ALogEntry::Type channel);
  void Push(const ALogEntry& entry);
  void _Show(bool show);
  bool _IsShown();
  void _GetConfig(FLogConfig& cfg);
  void _SetConfig(const FLogConfig& cfg);
  void UpdateWindow();

private:
  std::recursive_mutex WLocker;
  LogWindow* Window = nullptr;
  std::recursive_mutex ELocker;
  std::vector<ALogEntry> Entries;

  FIntPoint LastPosition;
  FIntPoint LastSize;
};