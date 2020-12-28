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
  enum class ActorClass : uint32
  {
    None = 0,
    StaticMeshes = 0x00000001,
    SkeletalMeshes = 0x00000002,
    Terrains = 0x00000004,
    SpeedTrees = 0x00000008,
    Prefabs = 0x00000010,
    Interps = 0x00000020,
    PointLights = 0x00000040,
    SpotLights = 0x00000080,
    DirectionalLights = 0x00000100,
    SkyLights = 0x00000200,
    HeightFog = 0x00000400,
    Emitters = 0x00000800,
    Sounds = 0x00001000,
    All = 0xFFFFFFFF
  };

  enum ConfigKey : uint16 {
    CFG_RootDir = 1,
    CFG_ActorMask,
    CFG_TerrainResample,
    CFG_SplitTerrainWeights,
    CFG_InvSqrtFalloff,
    CFG_SpotLightMul,
    CFG_PointLightMul,
    CFG_Material,
    CFG_Textures,
    CFG_TexturesFormat,
    CFG_Override,
    CFG_Lods,
    CFG_RBCollisions,
    CFG_IgnoreHidden,
    CFG_SplitT3D,
    CFG_MLods,
    CFG_DynamicShadows,
    CFG_LightmapUVs,
    CFG_End = 0xFFFF
  };

  FString RootDir;
  uint32 ActorClasses = ((uint32)ActorClass::All & ~(uint32)ActorClass::Sounds);

  // General
  bool OverrideData = false;
  bool IgnoreHidden = true;
  bool SplitT3D = false;
  
  // Materials
  bool Materials = true;
  bool Textures = true;
  int32 TextureFormat = 0;

  // Lights
  float SpotLightMul = 1.;
  float PointLightMul = 1.;
  float DirectionalLightMul = 1.; // Not in UI
  float SkyLightMul = 1.; // Not in UI
  bool InvSqrtFalloff = false;
  bool ForceDynamicShadows = true;

  // Terrains
  bool ResampleTerrain = true;
  bool SplitTerrainWeights = true;
  
  // Models
  bool ExportLods = false;
  bool ExportMLods = false;
  bool ConvexCollisions = true;
  bool ExportLightmapUVs = false;
  

  bool GetClassEnabled(ActorClass classFlag) const
  {
    return GetClassEnabled((uint32)(classFlag));
  }

  bool GetClassEnabled(uint32 classFlag) const
  {
    return ActorClasses & classFlag;
  }

  void SetClassEnabled(uint32 classFlag, bool on = true)
  {
    if (on)
    {
      ActorClasses |= classFlag;
    }
    else
    {
      ActorClasses &= ~classFlag;
    }
  }

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
    CFG_LastExport,
    CFG_LastImport,
    CFG_LastPkgOpen,
    CFG_LastPkgSave,

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

  FString LastExportPath;
  FString LastImportPath;
  FString LastPkgOpenPath;
  FString LastPkgSavePath;


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