#include "AConfiguration.h"
#include <Tera\FStream.h>

#define SerializeKey(key) { uint16 k = key; s << k; } //
#define SerializeKeyValue(key, value) { uint16 k = key; s << k << value; } //
#define CheckKey(key) { if (!s.IsGood()) { return s; } uint16 k = key; s << k; if (k != key) { s.Close(); return s; }} //

#define SerializeKVIfNotDefault(key, value, def) if (value != def) SerializeKeyValue(key, value) //

AConfiguration::AConfiguration(const std::string& path)
  : Path(path)
{}

bool AConfiguration::Load()
{
  FStream* s = new FReadStream(Path);
  uint32 size = (uint32)s->GetSize();
  if (!s->IsGood() || !size)
  {
    delete s;
    return false;
  }

  (*s) << Config;
  
  bool result = s->IsGood();
  delete s;
  return result;
}

bool AConfiguration::Save()
{
  FStream* s = new FWriteStream(Path);
  if (!s->IsGood())
  {
    delete s;
    return false;
  }

  (*s) << Config;

  delete s;
  return true;
}

FAppConfig AConfiguration::GetDefaultConfig() const
{
  return FAppConfig();
}

FAppConfig AConfiguration::GetConfig() const
{
  return Config;
}

void AConfiguration::SetConfig(const FAppConfig& cfg)
{
  Config = cfg;
}

FStream& operator<<(FStream& s, FLogConfig& c)
{
  if (s.IsReading())
  {
    while (s.IsGood())
    {
      uint16 key = 0;
      s << key;
      switch (key)
      {
      case FLogConfig::CFG_ShowLog:
        s << c.ShowLog;
        break;
      case FLogConfig::CFG_LogRect:
        s << c.LogRect;
        break;
      case FLogConfig::CFG_End:
        return s;
      default:
        s.Close();
        return s;
      }
    }
  }
  else
  {
    SerializeKeyValue(FLogConfig::CFG_ShowLog, c.ShowLog);
    SerializeKeyValue(FLogConfig::CFG_LogRect, c.LogRect);

    // End
    SerializeKey(FLogConfig::CFG_End);
  }
  return s;
}

FStream& operator<<(FStream& s, FMapExportConfig& c)
{
  if (s.IsReading())
  {
    while (s.IsGood())
    {
      uint16 key = 0;
      s << key;
      switch (key)
      {
      case FMapExportConfig::CFG_RootDir:
        s << c.RootDir;
        break;
      case FMapExportConfig::CFG_ActorMask:
        s << c.ActorClasses;
        break;
      // General
      case FMapExportConfig::CFG_Override:
        s << c.OverrideData;
        break;
      case FMapExportConfig::CFG_IgnoreHidden:
        s << c.IgnoreHidden;
        break;
      case FMapExportConfig::CFG_SplitT3D:
        s << c.SplitT3D;
        break;
      // Materials
      case FMapExportConfig::CFG_Material:
        s << c.Materials;
        break;
      case FMapExportConfig::CFG_Textures:
        s << c.Textures;
        break;
      case FMapExportConfig::CFG_TexturesFormat:
        s << c.TextureFormat;
        break;
      // Lights
      case FMapExportConfig::CFG_SpotLightMul:
        s << c.SpotLightMul;
        break;
      case FMapExportConfig::CFG_PointLightMul:
        s << c.PointLightMul;
        break;
      case FMapExportConfig::CFG_InvSqrtFalloff:
        s << c.InvSqrtFalloff;
        break;
      // Terrain
      case FMapExportConfig::CFG_TerrainResample:
        s << c.ResampleTerrain;
        break;
      case FMapExportConfig::CFG_SplitTerrainWeights:
        s << c.SplitTerrainWeights;
        break;
      // Models
      case FMapExportConfig::CFG_Lods:
        s << c.ExportLods;
        break;
      case FMapExportConfig::CFG_RBCollisions:
        s << c.ConvexCollisions;
        break;
      case FMapExportConfig::CFG_MLods:
        s << c.ExportMLods;
        break;
      case FMapExportConfig::CFG_DynamicShadows:
        s << c.ForceDynamicShadows;
        break;
      case FMapExportConfig::CFG_LightmapUVs:
        s << c.ExportLightmapUVs;
        break;
      default:
        s.Close();
        // no break
      case FMapExportConfig::CFG_End:
        return s;
      }
    }
  }
  else
  {
    FMapExportConfig d;
    SerializeKVIfNotDefault(FMapExportConfig::CFG_RootDir, c.RootDir, d.RootDir);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_ActorMask, c.ActorClasses, d.ActorClasses);

    SerializeKVIfNotDefault(FMapExportConfig::CFG_Override, c.OverrideData, d.OverrideData);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_IgnoreHidden, c.IgnoreHidden, d.IgnoreHidden);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_SplitT3D, c.SplitT3D, d.SplitT3D);

    SerializeKVIfNotDefault(FMapExportConfig::CFG_Material, c.Materials, d.Materials);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_Textures, c.Textures, d.Textures);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_TexturesFormat, c.TextureFormat, d.TextureFormat);

    SerializeKVIfNotDefault(FMapExportConfig::CFG_SpotLightMul, c.SpotLightMul, d.SpotLightMul);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_PointLightMul, c.PointLightMul, d.PointLightMul);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_InvSqrtFalloff, c.InvSqrtFalloff, d.InvSqrtFalloff);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_DynamicShadows, c.ForceDynamicShadows, d.ForceDynamicShadows);
    
    SerializeKVIfNotDefault(FMapExportConfig::CFG_TerrainResample, c.ResampleTerrain, d.ResampleTerrain);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_SplitTerrainWeights, c.SplitTerrainWeights, d.SplitTerrainWeights);
    
    SerializeKVIfNotDefault(FMapExportConfig::CFG_Lods, c.ExportLods, d.ExportLods);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_RBCollisions, c.ConvexCollisions, d.ConvexCollisions);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_MLods, c.ExportMLods, d.ExportMLods);
    SerializeKVIfNotDefault(FMapExportConfig::CFG_LightmapUVs, c.ExportLightmapUVs, d.ExportLightmapUVs);
    
    SerializeKey(FMapExportConfig::CFG_End);
  }
  return s;
}

FStream& operator<<(FStream& s, FAppConfig& c)
{
  s << c.Magic;
  if (c.Magic != PACKAGE_MAGIC)
  {
    s.Close();
  }
  s << c.Version;
  s << c.Size;
  if (s.IsReading())
  {
    uint16 key = 0;
    while (s.GetPosition() < (FILE_OFFSET)c.Size)
    {
      s << key;
      switch (key)
      {
      case FAppConfig::CFG_RootDir:
        s << c.RootDir;
        break;
      case FAppConfig::CFG_WindowRect:
        s << c.WindowRect;
        break;
      case FAppConfig::CFG_SashPos:
        s << c.SashPos;
        break;
      case FAppConfig::CFG_CompositeDumpPath:
        s << c.CompositeDumpPath;
        break;
      case FAppConfig::CFG_LastModAuthor:
        s << c.LastModAuthor;
        break;
      case FAppConfig::CFG_LastExport:
        s << c.LastExportPath;
        break;
      case FAppConfig::CFG_LastImport:
        s << c.LastImportPath;
        break;
      case FAppConfig::CFG_LastPkgOpen:
        s << c.LastPkgOpenPath;
        break;
      case FAppConfig::CFG_LastPkgSave:
        s << c.LastPkgSavePath;
        break;
      case FAppConfig::CFG_LogBegin:
        s << c.LogConfig;
        CheckKey(FAppConfig::CFG_LogEnd);
        break;
      case FAppConfig::CFG_MapExportBegin:
        s << c.MapExportConfig;
        CheckKey(FAppConfig::CFG_MapExportEnd);
        break;
      case FAppConfig::CFG_End:
        return s;
      default:
        s.Close();
        return s;
      }
    }
  }
  else
  {
    // Writing
    // General
    SerializeKeyValue(FAppConfig::CFG_RootDir, c.RootDir);
    SerializeKeyValue(FAppConfig::CFG_WindowRect, c.WindowRect);
    SerializeKeyValue(FAppConfig::CFG_SashPos, c.SashPos);
    SerializeKeyValue(FAppConfig::CFG_CompositeDumpPath, c.CompositeDumpPath);
    SerializeKeyValue(FAppConfig::CFG_LastModAuthor, c.LastModAuthor);

    SerializeKeyValue(FAppConfig::CFG_LastModAuthor, c.LastExportPath);
    SerializeKeyValue(FAppConfig::CFG_LastModAuthor, c.LastImportPath);
    SerializeKeyValue(FAppConfig::CFG_LastModAuthor, c.LastPkgOpenPath);
    SerializeKeyValue(FAppConfig::CFG_LastModAuthor, c.LastPkgSavePath);

    // Log
    SerializeKey(FAppConfig::CFG_LogBegin);
    s << c.LogConfig;
    SerializeKey(FAppConfig::CFG_LogEnd);

    // MapExport
    SerializeKey(FAppConfig::CFG_MapExportBegin);
    s << c.MapExportConfig;
    SerializeKey(FAppConfig::CFG_MapExportEnd);

    // End
    SerializeKey(FAppConfig::CFG_End);

    // Fixup storage size
    c.Size = s.GetSize();
    s.SetPosition(8);
    s << c.Size;
  }
  return s;
}
