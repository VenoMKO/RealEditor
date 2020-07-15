#pragma once
#include "Core.h"
#include "FStructs.h"
#include <fstream>

// Logger config
struct FLogConfig
{
  enum ConfigKey : uint16 {
    CFG_ShowLog = 1,
    CFG_LogPos,
    CFG_LogSize,
    CFG_End = 0xFFFF
  };

  // CFG_ShowLog: Show logger on startup
  bool ShowLog = false;
  // CFG_LogPos: Last window position
  FVector2D LogPosition = { -1, -1 };
  // CFG_LogSize: Last window size
  FVector2D LogSize = { 700, 300 };

  friend FStream& operator<<(FStream& s, FLogConfig& c);
};

// Application config
struct FAppConfig
{
  enum ConfigKey : uint16 {
    CFG_RootDir = 1,
    CFG_LogBegin = 100,
    CFG_LogEnd,
    CFG_End = 0xFFFF
  };
  uint32 Magic = PACKAGE_MAGIC;
  uint16 Version = 1;
  uint32 Size = 0;
  
  // CFG_RootDir: CookedPC path(UTF8)
  std::string RootDir;
  // CFG_LogBegin: Logger config
  FLogConfig LogConfig;

  friend FStream& operator<<(FStream& s, FAppConfig& c);
};

// Application config serializer
class AConfiguration {
public:
  // path - path to the config file
  AConfiguration(const std::string& path);

  // Load config from the file. Returns false on error
  bool Load();

  // Save config to the file. Returns false on error
  bool Save();
  
  FAppConfig GetConfig() const;
  void SetConfig(const FAppConfig& cfg);

  // Get a default config
  FAppConfig GetDefaultConfig() const;

private:
  std::string Path;
  FAppConfig Config;
};