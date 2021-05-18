#pragma once
#include <Tera\Core.h>
#include <Tera\FStructs.h>

#include <fstream>
#include <algorithm>

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

struct FSkelMeshExportConfig {
  enum ConfigKey : uint16 {
    CFG_ExportMode = 1,
    CFG_ExportTextures,
    CFG_ScaleFactor,
    CFG_TextureFormat,
    CFG_End = 0xFFFF
  };

  int32 Mode = 0;
  int32 TextureFormat = 0;
  bool ExportTextures = true;
  float ScaleFactor = 1.;

  friend FStream& operator<<(FStream& s, FSkelMeshExportConfig& c);
};

struct FSkelMeshImportConfig {
  enum ConfigKey : uint16 {
    CFG_ImportSkel = 1,
    CFG_ImportTangents,
    CFG_FlipTangentY,
    CFG_TangentYByUV,
    CFG_AverageTangentZ,
    CFG_OptimizeIndices,
    CFG_End = 0xFFFF
  };

  bool ImportSkeleton = false;
  bool ImportTangents = true;
  bool FlipTangentY = false;
  bool TangentYBasisByUV = true;
  bool AverageTangentZ = false;
  bool OptimizeIndexBuffer = true;

  friend FStream& operator<<(FStream& s, FSkelMeshImportConfig& c);
};

struct FStaticMeshExportConfig {
  enum ConfigKey : uint16 {
    CFG_ExportTextures = 1,
    CFG_ScaleFactor,
    CFG_TextureFormat,
    CFG_End = 0xFFFF
  };

  int32 TextureFormat = 0;
  bool ExportTextures = true;
  float ScaleFactor = 1.;

  friend FStream& operator<<(FStream& s, FStaticMeshExportConfig& c);
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
    CFG_GlobalScale,
    CFG_End = 0xFFFF
  };

  FString RootDir;
  uint32 ActorClasses = ((uint32)ActorClass::All & ~(uint32)ActorClass::Sounds);

  // General
  bool OverrideData = false;
  bool IgnoreHidden = true;
  bool SplitT3D = false;
  float GlobalScale = 4.f;
  
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
    CFG_LastPkgOpen/*legacy, replaced with CFG_LastFilePackages*/,
    CFG_LastPkgSave,
    CFG_MaxLastFilePackages,
    CFG_LastFilePackages,
    CFG_LastTextureExtension,
    CFG_SavePackageDontAskAgain,

    // Log
    CFG_LogBegin = 100,
    CFG_LogEnd,

    // MapExport
    CFG_MapExportBegin = 110,
    CFG_MapExportEnd,

    // SkelMeshExport
    CFG_SkelMeshExportBegin = 120,
    CFG_SkelMeshExportEnd,

    // StaticMeshExport
    CFG_StaticMeshExportBegin = 130,
    CFG_StaticMeshExportEnd,

    // SkelMeshImport
    CFG_SkelMeshImportBegin = 140,
    CFG_SkelMeshImportEnd,

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
  // CFG_LastExport: last export dialog path
  FString LastExportPath;
  // CFG_LastImport: last import dialog path
  FString LastImportPath;
  // CFG_LastPkgSavePath: last path used to save a GPK
  FString LastPkgSavePath;
  // CFG_MaxLastFilePackages: number of maximum entries to store in LastFilePackages
  int32 MaxLastFilePackages = 10;
  // CFG_LastFilePackages: list of opened GPK files. New entries add to the beginning of the vector.
  std::vector<FString> LastFilePackages;
  // CFG_LastTextureExtension: last file extension used to save a texture
  uint8 LastTextureExtension = 0;
  // CFG_SavePackageDontAskAgain: Save package warning
  bool SavePackageDontShowAgain = false;

  // Fast accessor to the last opened GPK file path
  FString GetLastFilePackagePath() const
  {
    return LastFilePackages.size() ? LastFilePackages.front() : FString();
  }

  // Add a path to the last opened GPK files
  void AddLastFilePackagePath(const FString& path)
  {
    auto it = std::find(LastFilePackages.begin(), LastFilePackages.end(), path);
    if (it != LastFilePackages.end())
    {
      LastFilePackages.erase(it);
    }
    LastFilePackages.insert(LastFilePackages.begin(), path);
    if (MaxLastFilePackages > 0 && (static_cast<int32>(LastFilePackages.size()) - MaxLastFilePackages) > 0)
    {
      // Remove old entries
      LastFilePackages.erase(LastFilePackages.begin() + MaxLastFilePackages, LastFilePackages.end());
    }
  }

  // CFG_LogBegin: Logger config
  FLogConfig LogConfig;

  // CFG_MapExportBegin: Map export settings
  FMapExportConfig MapExportConfig;

  // CFG_SkelMeshExportBegin: SkelMesh export settings
  FSkelMeshExportConfig SkelMeshExportConfig;

  // CFG_SkelMeshImportBegin: SkelMesh export settings
  FSkelMeshImportConfig SkelMeshImportConfig;

  // CFG_StaticMeshExportBegin: StaticMesh export settings
  FStaticMeshExportConfig StaticMeshExportConfig;

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