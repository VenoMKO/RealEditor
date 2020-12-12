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

struct FMapExportConfig {
  enum ConfigKey : uint16 {
    CFG_RootDir = 1,
    CFG_Terrains,
    CFG_Statics,
    CFG_Skeletals,
    CFG_SpeedTrees,
    CFG_PointLights,
    CFG_SpotLights,
    CFG_Interps,
    CFG_Emitters,
    CFG_Sounds,
    CFG_TerrainResample,
    CFG_SplitTerrainWeights,
    CFG_InvSqrtFalloff,
    CFG_SpotLightMul,
    CFG_PointLightMul,
    CFG_Material,
    CFG_Textures,
    CFG_TexturesFormat,
    CFG_End = 0xFFFF
  };

  FString RootDir;
  bool Terrains = true;
  bool Statics = true;
  bool Skeletals = true;
  bool SpeedTrees = true;
  bool PointLights = true;
  bool SpotLights = true;
  bool Interps = true;
  bool Emitters = true;
  bool Sounds = true;
  bool ResampleTerrain = true;
  bool SplitTerrainWeights = true;
  bool InvSqrtFalloff = false;
  float SpotLightMul = 1.;
  float PointLightMul = 1.;
  bool Materials = true;
  bool Textures = true;
  int32 TextureFormat = 0;

  friend FStream& operator<<(FStream& s, FMapExportConfig& c);
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

    // MapExport
    CFG_MapExportBegin = 110,
    CFG_MapExportEnd,

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

  // CFG_MapExportBegin: Map export settings
  FMapExportConfig MapExportConfig;

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