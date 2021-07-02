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
      case FMapExportConfig::CFG_GlobalScale:
        s << c.GlobalScale;
        break;
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

    SerializeKVIfNotDefault(FMapExportConfig::CFG_GlobalScale, c.GlobalScale, d.GlobalScale);
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

void SerializeVersion(FStream& s, FAppConfig& c)
{
  float oldVersion = 0;
  // Placeholder for backwards compatibility. Older RE will read the placeholder as
  // config size. Zero size won't crash or corrupt older RE, but will reset the config to its defaults.
  uint32 placeholder = 0;
  if (s.IsReading())
  {
    s << oldVersion;
    if (oldVersion < 0.)
    {
      s << placeholder;
      s << c.VerMajor;
      s << c.VerMinor;
      s << c.BuildNum;
    }
    else
    {
      // Old config stored version as float
      c.VerMajor = floor(oldVersion);
      float intp = 0;
      float fracp = modf(oldVersion, &intp);
      c.VerMajor = (uint16)intp;
      c.VerMinor = (uint16)(fracp * 100.);
      c.BuildNum = 0;
    }
  }
  else
  {
    c.VerMajor = APP_VER_MAJOR;
    c.VerMinor = APP_VER_MINOR;
    c.BuildNum = BUILD_NUMBER;
    oldVersion = -1.;
    s << oldVersion;
    s << placeholder;
    s << c.VerMajor;
    s << c.VerMinor;
    s << c.BuildNum;
  }
}

FStream& operator<<(FStream& s, FAppConfig& c)
{
  FAppConfig d;
  s << c.Magic;
  if (c.Magic != PACKAGE_MAGIC)
  {
    s.Close();
  }
  SerializeVersion(s, c);
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
      case FAppConfig::CFG_MaxLastFilePackages:
      {
        s << c.MaxLastFilePackages;
        if (c.MaxLastFilePackages != d.MaxLastFilePackages)
        {
          c.MaxLastFilePackages = d.MaxLastFilePackages;
        }
        break;
      }
      case FAppConfig::CFG_LastFilePackages:
        s << c.LastFilePackages;
        break;
      case FAppConfig::CFG_LastTextureExtension:
        s << c.LastTextureExtension;
        break;
      case FAppConfig::CFG_LogBegin:
        s << c.LogConfig;
        CheckKey(FAppConfig::CFG_LogEnd);
        break;
      case FAppConfig::CFG_MapExportBegin:
        s << c.MapExportConfig;
        CheckKey(FAppConfig::CFG_MapExportEnd);
        break;
      case FAppConfig::CFG_SkelMeshExportBegin:
        s << c.SkelMeshExportConfig;
        CheckKey(FAppConfig::CFG_SkelMeshExportEnd);
        break;
      case FAppConfig::CFG_SkelMeshImportBegin:
        s << c.SkelMeshImportConfig;
        CheckKey(FAppConfig::CFG_SkelMeshImportEnd);
        break;
      case FAppConfig::CFG_StaticMeshExportBegin:
        s << c.StaticMeshExportConfig;
        CheckKey(FAppConfig::CFG_StaticMeshExportEnd);
        break;
      case FAppConfig::CFG_AnimationExportBegin:
        s << c.AnimationExportConfig;
        CheckKey(FAppConfig::CFG_AnimationExportEnd);
        break;
      case FAppConfig::CFG_SavePackageDontAskAgain:
        s << c.SavePackageDontShowAgain;
        break;
      case FAppConfig::CFG_SavePackageOpenDontAskAgain:
        s << c.SavePackageOpenDontAskAgain;
        break;
      case FAppConfig::CFG_SavePackageOpen:
        s << c.SavePackageOpen;
        break;
      case FAppConfig::CFG_BulkImportTfcMode:
        s << c.BulkImportTfcMode;
        break;
      case FAppConfig::CFG_LastDcKey:
        s << c.LastDcKey;
        break;
      case FAppConfig::CFG_LastDcVec:
        s << c.LastDcVec;
        break;
      case FAppConfig::CFG_LastDcMode:
        s << c.LastDcMode;
        break;
      case FAppConfig::CFG_LastDcPath:
        s << c.LastDcPath;
        break;
      case FAppConfig::CFG_LastDcSavePath:
        s << c.LastDcSavePath;
        break;
      case FAppConfig::CFG_FastObjectDump:
        s << c.FastObjectDump;
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
    FILE_OFFSET sizeOffset = s.GetPosition() - (FILE_OFFSET)sizeof(c.Size);
    // Writing
    // General
    SerializeKeyValue(FAppConfig::CFG_RootDir, c.RootDir);
    SerializeKeyValue(FAppConfig::CFG_WindowRect, c.WindowRect);
    SerializeKeyValue(FAppConfig::CFG_SashPos, c.SashPos);
    SerializeKeyValue(FAppConfig::CFG_CompositeDumpPath, c.CompositeDumpPath);
    SerializeKeyValue(FAppConfig::CFG_LastModAuthor, c.LastModAuthor);

    SerializeKeyValue(FAppConfig::CFG_LastExport, c.LastExportPath);
    SerializeKeyValue(FAppConfig::CFG_LastImport, c.LastImportPath);

    SerializeKeyValue(FAppConfig::CFG_LastPkgOpen, c.LastPkgOpenPath);
    SerializeKeyValue(FAppConfig::CFG_LastPkgSave, c.LastPkgSavePath);

    SerializeKeyValue(FAppConfig::CFG_MaxLastFilePackages, c.MaxLastFilePackages);
    SerializeKeyValue(FAppConfig::CFG_LastFilePackages, c.LastFilePackages);
    SerializeKeyValue(FAppConfig::CFG_LastTextureExtension, c.LastTextureExtension);

    SerializeKeyValue(FAppConfig::CFG_SavePackageDontAskAgain, c.SavePackageDontShowAgain);

    SerializeKeyValue(FAppConfig::CFG_SavePackageOpen, c.SavePackageOpen);
    SerializeKeyValue(FAppConfig::CFG_SavePackageOpenDontAskAgain, c.SavePackageOpenDontAskAgain);
    SerializeKeyValue(FAppConfig::CFG_BulkImportTfcMode, c.BulkImportTfcMode);

    SerializeKeyValue(FAppConfig::CFG_LastDcKey, c.LastDcKey);
    SerializeKeyValue(FAppConfig::CFG_LastDcVec, c.LastDcVec);
    SerializeKeyValue(FAppConfig::CFG_LastDcMode, c.LastDcMode);
    SerializeKeyValue(FAppConfig::CFG_LastDcPath, c.LastDcPath);
    SerializeKeyValue(FAppConfig::CFG_LastDcSavePath, c.LastDcSavePath);
    SerializeKeyValue(FAppConfig::CFG_FastObjectDump, c.FastObjectDump);

    // Log
    SerializeKey(FAppConfig::CFG_LogBegin);
    s << c.LogConfig;
    SerializeKey(FAppConfig::CFG_LogEnd);

    // MapExport
    SerializeKey(FAppConfig::CFG_MapExportBegin);
    s << c.MapExportConfig;
    SerializeKey(FAppConfig::CFG_MapExportEnd);

    // SkelMeshExport
    SerializeKey(FAppConfig::CFG_SkelMeshExportBegin);
    s << c.SkelMeshExportConfig;
    SerializeKey(FAppConfig::CFG_SkelMeshExportEnd);

    // SkelMeshImport
    SerializeKey(FAppConfig::CFG_SkelMeshImportBegin);
    s << c.SkelMeshImportConfig;
    SerializeKey(FAppConfig::CFG_SkelMeshImportEnd);

    // StaticMeshExport
    SerializeKey(FAppConfig::CFG_StaticMeshExportBegin);
    s << c.StaticMeshExportConfig;
    SerializeKey(FAppConfig::CFG_StaticMeshExportEnd);

    // AnimationsExport
    SerializeKey(FAppConfig::CFG_AnimationExportBegin);
    s << c.AnimationExportConfig;
    SerializeKey(FAppConfig::CFG_AnimationExportEnd);

    // End
    SerializeKey(FAppConfig::CFG_End);

    // Fixup storage size
    c.Size = s.GetSize();
    s.SetPosition(sizeOffset);
    s << c.Size;
  }
  return s;
}

FStream& operator<<(FStream& s, FSkelMeshExportConfig& c)
{
  if (s.IsReading())
  {
    while (s.IsGood())
    {
      uint16 key = 0;
      s << key;
      switch (key)
      {
      case FSkelMeshExportConfig::CFG_ExportMode:
        s << c.Mode;
        break;
      case FSkelMeshExportConfig::CFG_ExportTextures:
        s << c.ExportTextures;
        break;
      case FSkelMeshExportConfig::CFG_ScaleFactor:
        s << c.ScaleFactor;
        break;
      case FSkelMeshExportConfig::CFG_TextureFormat:
        s << c.TextureFormat;
        break;
      default:
        s.Close();
        // no break
      case FSkelMeshExportConfig::CFG_End:
        return s;
      }
    }
  }
  else
  {
    FSkelMeshExportConfig d;
    SerializeKVIfNotDefault(FSkelMeshExportConfig::CFG_ExportMode, c.Mode, d.Mode);
    SerializeKVIfNotDefault(FSkelMeshExportConfig::CFG_ExportTextures, c.ExportTextures, d.ExportTextures);
    SerializeKVIfNotDefault(FSkelMeshExportConfig::CFG_ScaleFactor, c.ScaleFactor, d.ScaleFactor);
    SerializeKVIfNotDefault(FSkelMeshExportConfig::CFG_TextureFormat, c.TextureFormat, d.TextureFormat);
    SerializeKey(FSkelMeshExportConfig::CFG_End);
  }
  return s;
}

FStream& operator<<(FStream& s, FStaticMeshExportConfig& c)
{
  if (s.IsReading())
  {
    while (s.IsGood())
    {
      uint16 key = 0;
      s << key;
      switch (key)
      {
      case FStaticMeshExportConfig::CFG_ExportTextures:
        s << c.ExportTextures;
        break;
      case FStaticMeshExportConfig::CFG_ScaleFactor:
        s << c.ScaleFactor;
        break;
      case FStaticMeshExportConfig::CFG_TextureFormat:
        s << c.TextureFormat;
        break;
      default:
        s.Close();
        // no break
      case FStaticMeshExportConfig::CFG_End:
        return s;
      }
    }
  }
  else
  {
    FStaticMeshExportConfig d;
    SerializeKVIfNotDefault(FStaticMeshExportConfig::CFG_ExportTextures, c.ExportTextures, d.ExportTextures);
    SerializeKVIfNotDefault(FStaticMeshExportConfig::CFG_ScaleFactor, c.ScaleFactor, d.ScaleFactor);
    SerializeKVIfNotDefault(FStaticMeshExportConfig::CFG_TextureFormat, c.TextureFormat, d.TextureFormat);
    SerializeKey(FStaticMeshExportConfig::CFG_End);
  }
  return s;
}

FStream& operator<<(FStream& s, FSkelMeshImportConfig& c)
{
  if (s.IsReading())
  {
    while (s.IsGood())
    {
      uint16 key = 0;
      s << key;
      switch (key)
      {
      case FSkelMeshImportConfig::CFG_ImportSkel:
        s << c.ImportSkeleton;
        break;
      case FSkelMeshImportConfig::CFG_ImportTangents:
        s << c.ImportTangents;
        break;
      case FSkelMeshImportConfig::CFG_FlipTangentY:
        s << c.FlipTangentY;
        break;
      case FSkelMeshImportConfig::CFG_TangentYByUV:
        s << c.TangentYBasisByUV;
        break;
      case FSkelMeshImportConfig::CFG_AverageTangentZ:
        s << c.AverageTangentZ;
        break;
      case  FSkelMeshImportConfig::CFG_OptimizeIndices:
        s << c.OptimizeIndexBuffer;
        break;
      default:
        s.Close();
        // no break
      case FSkelMeshImportConfig::CFG_End:
        return s;
      }
    }
  }
  else
  {
    FSkelMeshImportConfig d;
    SerializeKVIfNotDefault(FSkelMeshImportConfig::CFG_ImportSkel, c.ImportSkeleton, d.ImportSkeleton);
    SerializeKVIfNotDefault(FSkelMeshImportConfig::CFG_ImportTangents, c.ImportTangents, d.ImportTangents);
    SerializeKVIfNotDefault(FSkelMeshImportConfig::CFG_FlipTangentY, c.FlipTangentY, d.FlipTangentY);
    SerializeKVIfNotDefault(FSkelMeshImportConfig::CFG_TangentYByUV, c.TangentYBasisByUV, d.TangentYBasisByUV);
    SerializeKVIfNotDefault(FSkelMeshImportConfig::CFG_AverageTangentZ, c.AverageTangentZ, d.AverageTangentZ);
    SerializeKVIfNotDefault(FSkelMeshImportConfig::CFG_OptimizeIndices, c.OptimizeIndexBuffer, d.OptimizeIndexBuffer);
    SerializeKey(FSkelMeshImportConfig::CFG_End);
  }
  return s;
}

FStream& operator<<(FStream& s, FAnimationExportConfig& c)
{
  if (s.IsReading())
  {
    while (s.IsGood())
    {
      uint16 key = 0;
      s << key;
      switch (key)
      {
      case FAnimationExportConfig::CFG_Mesh:
        s << c.ExportMesh;
        break;
      case FAnimationExportConfig::CFG_Compress:
        s << c.Compress;
        break;
      case FAnimationExportConfig::CFG_RateFactor:
        s << c.RateFactor;
        break;
      case FAnimationExportConfig::CFG_Resample:
        s << c.Resample;
        break;
      case FAnimationExportConfig::CFG_ScaleFactor:
        s << c.ScaleFactor;
        break;
      case FAnimationExportConfig::CFG_Split:
        s << c.Split;
        break;
      default:
        s.Close();
        // no break
      case FAnimationExportConfig::CFG_End:
        return s;
      }
    }
  }
  else
  {
    FAnimationExportConfig d;
    SerializeKVIfNotDefault(FAnimationExportConfig::CFG_Mesh, c.ExportMesh, d.ExportMesh);
    SerializeKVIfNotDefault(FAnimationExportConfig::CFG_Compress, c.Compress, d.Compress);
    SerializeKVIfNotDefault(FAnimationExportConfig::CFG_RateFactor, c.RateFactor, d.RateFactor);
    SerializeKVIfNotDefault(FAnimationExportConfig::CFG_Resample, c.Resample, d.Resample);
    SerializeKVIfNotDefault(FAnimationExportConfig::CFG_ScaleFactor, c.ScaleFactor, d.ScaleFactor);
    SerializeKVIfNotDefault(FAnimationExportConfig::CFG_Split, c.Split, d.Split);
    SerializeKey(FAnimationExportConfig::CFG_End);
  }
  return s;
}
