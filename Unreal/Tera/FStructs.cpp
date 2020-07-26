#include "FStructs.h"
#include "FStream.h"

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
  s << e.Package;
  s << e.ObjectPath;
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

  s << sum.FileVersion << sum.LicenseeVersion;
  if (s.IsReading() && (sum.FileVersion != 610 && sum.FileVersion != 897))
  {
    UThrow("Package version \"" + std::to_string(sum.FileVersion) + "/" + std::to_string(sum.LicenseeVersion) + "\" is not supported.");
  }

  s << sum.HeaderSize;
  s << sum.FolderName;
  s << sum.PackageFlags;

  if (sum.LicenseeVersion == 14)
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
  if (sum.FileVersion == 897)
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
  if (sum.FileVersion == 897)
  {
    s << sum.TextureAllocations;
  }
  return s;
}

FStream& operator<<(FStream& s, FVector2D& v)
{
  return s << v.X << v.Y;
}
