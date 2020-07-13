#pragma once
#include "Core.h"
#include <fstream>

// Application config default structure
struct FConfig
{
  uint32 Magic = PACKAGE_MAGIC;
  uint16 Version = 1;
  uint32 Size = 0;
  
  // CFG_RootDir: CookedPC path(UTF8)
  std::string RootDir;

  friend FStream& operator<<(FStream& s, FConfig& c);
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
  
  FConfig GetConfig() const;
  void SetConfig(const FConfig& cfg);

  // Get a default config
  FConfig GetDefaultConfig() const;

private:
  std::string Path;
  FConfig Config;
};