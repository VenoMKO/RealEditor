#pragma once
#include <Tera\Core.h>
#include <Tera\FStructs.h>

#include <fstream>

// Logger config
struct FLogConfig
{
  enum ConfigKey : uint16 {
    CFG_ShowLog = 1,
    CFG_LogRect,
    CFG_End = 0xFFFF
  };

  // CFG_ShowLog: Show logger on startup
  bool ShowLog = false;
  // CFG_LogPos: Last window position
  FIntRect LogRect = { { -1, -1 }, { 700, 300 } };

  friend FStream& operator<<(FStream& s, FLogConfig& c);
};

// Application config
struct FAppConfig
{
  enum ConfigKey : uint16 {
    // General
    CFG_RootDir = 1,
    CFG_WindowRect,
    CFG_SashPos,
    CFG_CompositeDumpPath,
    CFG_LastModAuthor,

    // Log
    CFG_LogBegin = 100,
    CFG_LogEnd,
    CFG_End = 0xFFFF
  };
  uint32 Magic = PACKAGE_MAGIC;
  float Version = APP_VER;
  uint32 Size = 0;
  
  // CFG_RootDir: CookedPC path(UTF8)
  FString RootDir;
  // CFG_WindowRect: Window. Min - Origin, Max - Size
  FIntRect WindowRect = { { WIN_POS_CENTER, 0 }, { 1024, 700 } };
  // CFG_SashPos: Sash positions. Min - ObjTree, Max - Props. X - sash pos. Y - split width
  FIntRect SashPos = { { 230, 1008 }, { 540, 774 } };
  // CFG_CompositeDumpPath: Last composite dump location
  FString CompositeDumpPath;
  // CFG_LastModAuthor: Last composite mod author
  FString LastModAuthor;


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