#include "FStructs.h"
#include "FStream.h"
#include "UObject.h"

FStream& operator<<(FStream& s, FGuid& g)
{
  return s << g.A << g.B << g.C << g.D;
}

FStream& operator<<(FStream& s, FCompressedChunk& c)
{
  return s << c.DecompressedOffset << c.DecompressedSize << c.CompressedOffset << c.CompressedSize;
}

FStream& operator<<(FStream& s, FGeneration& g)
{
  return s << g.Exports << g.Names << g.NetObjects;
}

FStream& operator<<(FStream& s, FTextureAllocations::FTextureType& t)
{
  s << t.SizeX;
  s << t.SizeY;
  s << t.NumMips;
  s << t.Format;
  s << t.TexCreateFlags;
  s << t.ExportIndices;
  return s;
}

FStream& operator<<(FStream& s, FTextureAllocations& t)
{
  return s << t.TextureTypes;
}

FStream& operator<<(FStream& s, FCompositePackageMapEntry& e)
{
  s << e.ObjectPath;
  s << e.FileName;
  s << e.Offset;
  s << e.Size;
  return s;
}

FStream& operator<<(FStream& s, FPackageSummary& sum)
{
  s << sum.Magic;
  if (s.IsReading() && sum.Magic != PACKAGE_MAGIC)
  {
    UThrow("File is not a valid package.");
  }
  s << sum.FileVersion;
  const uint16 fv = sum.GetFileVersion();
  const uint16 lv = sum.GetLicenseeVersion();
  if (s.IsReading() && (fv != 610 && fv != 897))
  {
    UThrow("Package version \"%d/%d\" is not supported.", fv, lv);
  }

  s << sum.HeaderSize;
  s << sum.FolderName;
  s << sum.PackageFlags;

  if (fv == 610 && lv == 14)
  {
    if (!s.IsReading())
    {
      sum.NamesCount += sum.NamesOffset;
    }
    s << sum.NamesCount << sum.NamesOffset;
    sum.NamesCount -= sum.NamesOffset;
  }
  else
  {
    s << sum.NamesCount << sum.NamesOffset;
  }

  s << sum.ExportsCount << sum.ExportsOffset;
  s << sum.ImportsCount << sum.ImportsOffset;
  s << sum.DependsOffset;
  if (fv == 897)
  {
    s << sum.ImportExportGuidsOffset;
    s << sum.ImportGuidsCount;
    s << sum.ExportGuidsCount;
    s << sum.ThumbnailTableOffset;
  }
  s << sum.Guid;
  s << sum.Generations;
  s << sum.EngineVersion << sum.ContentVersion;
  s << sum.CompressionFlags;
  s << sum.CompressedChunks;
  s << sum.PackageSource;
  s << sum.AdditionalPackagesToCook;
  if (fv == 897)
  {
    s << sum.TextureAllocations;
  }
  return s;
}

FStream& operator<<(FStream& s, FVector2D& v)
{
  return s << v.X << v.Y;
}

inline FString FScriptDelegate::ToString(const UObject* OwnerObject) const
{
  const UObject* DelegateObject = Object;
  if (DelegateObject == NULL)
  {
    DelegateObject = OwnerObject;
  }
  return DelegateObject->GetObjectPath() + "." + FunctionName.String();
}

FStream& operator<<(FStream& s, FScriptDelegate& d)
{
  return s << d.Object << d.FunctionName;
}

FStream& operator<<(FStream& s, FPacakgeTreeEntry& e)
{
  s << e.FullObjectName;
  s << e.ClassName;
  return s;
}

FStream& operator<<(FStream& s, FCookedBulkDataInfo& i)
{
  s << i.SavedBulkDataFlags;
  s << i.SavedElementCount;
  s << i.SavedBulkDataOffsetInFile;
  s << i.SavedBulkDataSizeOnDisk;
  s << i.TextureFileCacheName;
  return s;
}

FStream& operator<<(FStream& s, FCookedTextureFileCacheInfo& i)
{
  s << i.TextureFileCacheGuid;
  s << i.TextureFileCacheName;
  s << i.LastSaved;
  return s;
}

FStream& operator<<(FStream& s, FCookedTextureUsageInfo& i)
{
  s << i.PackageNames;
  s << i.Format;
  s << i.LODGroup;
  s << i.SizeX << i.SizeY;
  s << i.StoredOnceMipSize;
  s << i.DuplicatedMipSize;
  return s;
}

FStream& operator<<(FStream& s, FForceCookedInfo& i)
{
  return s << i.CookedContentList;
}

FStream& operator<<(FStream& s, FSHA& i)
{
  s.SerializeBytes(i.Bytes, 20);
  return s;
}