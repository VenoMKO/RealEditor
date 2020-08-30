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

FStream& operator<<(FStream& s, FCompressedChunkInfo& c)
{
  return s << c.CompressedSize << c.DecompressedSize;
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
#if _DEBUG
    // UDK
    if (fv != 868)
#endif
    UThrow("Package version \"%hu/%hu\" is not supported.", fv, lv);
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
  if (fv > VER_TERA_CLASSIC)
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
  if (fv > VER_TERA_CLASSIC)
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
  SERIALIZE_UREF(s, d.Object);
  return s << d.FunctionName;
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

FStream& operator<<(FStream& s, AMetaDataEntry& e)
{
  return s << e.Name << e.Tooltip;
}

FStream& operator<<(FStream& s, FSHA& i)
{
  s.SerializeBytes(i.Bytes, 20);
  return s;
}

FStream& operator<<(FStream& s, FIntPoint& p)
{
  return s << p.X << p.Y;
}

FStream& operator<<(FStream& s, FIntRect& r)
{
  return s << r.Min << r.Max;
}

FStream& operator<<(FStream& s, FLevelGuids& g)
{
  return s << g.LevelName << g.Guids;
}

FStream& operator<<(FStream& s, FObjectThumbnailInfo& i)
{
  return s << i.ObjectClassName << i.ObjectPath << i.Offset;
}

FStream& operator<<(FStream& s, FObjectThumbnail& t)
{
  s << t.Width;
  s << t.Height;

  s << t.CompressedSize;
  if (s.IsReading())
  {
    if (t.CompressedSize)
    {
      t.CompressedData = malloc(t.CompressedSize);
    }
  }
  s.SerializeBytes(t.CompressedData, t.CompressedSize);
  return s;
}

void FUntypedBulkData::SerializeBulkData(FStream& s, void* data)
{
  if (BulkDataFlags & BULKDATA_Unused)
  {
    return;
  }

  bool bSerializeInBulk = true;
  if (RequiresSingleElementSerialization(s) || (BulkDataFlags & BULKDATA_ForceSingleElementSerialization) || (!s.IsReading() && (GetElementSize() > 1)))
  {
    bSerializeInBulk = false;
  }

  if (bSerializeInBulk)
  {
    if (BulkDataFlags & BULKDATA_SerializeCompressed)
    {
      s.SerializeCompressed(data, GetBulkDataSize(), GetDecompressionFlags());
    }
    else
    {
      s.SerializeBytes(data, GetBulkDataSize());
    }
  }
  else
  {
    if (BulkDataFlags & BULKDATA_SerializeCompressed)
    {
      uint8* serializedData = nullptr;
      if (s.IsReading())
      {
        OwnsMemory = true;
        serializedData = (uint8*)malloc(GetBulkDataSize());
        s.SerializeCompressed(serializedData, GetBulkDataSize(), GetDecompressionFlags());

        MReadStream memStream(serializedData, true, GetBulkDataSize());

        // Serialize each element individually via memory reader.				
        for (int32 idx = 0; idx < ElementCount; ++idx)
        {
          SerializeElement(memStream, data, idx);
        }
      }
      else
      {
        // TODO: compression
      }
    }
    else
    {
      // We can use the passed in archive if we're not compressing the data.
      for (int32 idx = 0; idx < ElementCount; ++idx)
      {
        SerializeElement(s, data, idx);
      }
    }
  }
}

bool FUntypedBulkData::RequiresSingleElementSerialization(FStream& s)
{
  return false;
}

void FUntypedBulkData::GetCopy(void** dest) const
{
  if (*dest)
  {
    if (BulkData)
    {
      memcpy(*dest, BulkData, GetBulkDataSize());
    }
  }
  else
  {
    if (BulkData)
    {
      *dest = malloc(GetBulkDataSize());
      memcpy(*dest, BulkData, GetBulkDataSize());
    }
  }
}

void FUntypedBulkData::Serialize(FStream& s, UObject* owner, int32 idx)
{
  s << BulkDataFlags;
  s << ElementCount;
  if (s.IsReading())
  {
    s << BulkDataSizeOnDisk;
    s << BulkDataOffsetInFile;

    if (!(BulkDataFlags & BULKDATA_StoreInSeparateFile))
    {
      OwnsMemory = true;
      BulkData = malloc(GetBulkDataSize());
      SerializeBulkData(s, BulkData);
    }
  }
  else
  {
    // TODO: compression
  }
}

void FUntypedBulkData::SerializeSeparate(FStream& s, UObject* owner, int32 idx)
{
  bool hasFlag = BulkDataFlags & BULKDATA_StoreInSeparateFile;
  if (hasFlag)
  {
    BulkDataFlags &= ~BULKDATA_StoreInSeparateFile;
  }
  OwnsMemory = true;
  BulkData = malloc(GetBulkDataSize());
  SerializeBulkData(s, BulkData);
  if (hasFlag)
  {
    BulkDataFlags |= BULKDATA_StoreInSeparateFile;
  }
}

FUntypedBulkData::~FUntypedBulkData()
{
  if (BulkData && OwnsMemory)
  {
    free(BulkData);
  }
}

int32 FUntypedBulkData::GetElementCount() const
{
  return ElementCount;
}

int32 FUntypedBulkData::GetBulkDataSize() const
{
  return GetElementCount() * GetElementSize();
}

int32 FUntypedBulkData::GetBulkDataSizeOnDisk() const
{
  return BulkDataSizeOnDisk;
}

int32 FUntypedBulkData::GetBulkDataOffsetInFile() const
{
  return BulkDataOffsetInFile;
}

bool FUntypedBulkData::IsStoredCompressedOnDisk() const
{
  return BulkDataFlags & BULKDATA_SerializeCompressed;
}

bool FUntypedBulkData::IsStoredInSeparateFile() const
{
  return BulkDataFlags & BULKDATA_StoreInSeparateFile;
}

ECompressionFlags FUntypedBulkData::GetDecompressionFlags() const
{
  return (BulkDataFlags & BULKDATA_SerializeCompressedZLIB) ? COMPRESS_ZLIB :
         (BulkDataFlags & BULKDATA_SerializeCompressedLZX) ? COMPRESS_LZX :
         (BulkDataFlags & BULKDATA_SerializeCompressedLZO) ? COMPRESS_LZO :
         COMPRESS_None;
}

int32 FByteBulkData::GetElementSize() const
{
  return 1;
}

void FByteBulkData::SerializeElement(FStream& s, void* data, int32 elementIndex)
{
  uint8& b = *((uint8*)data + elementIndex);
  s << b;
}

FTexture2DMipMap::~FTexture2DMipMap()
{
  delete Data;
}

void FTexture2DMipMap::Serialize(FStream& s, UObject* owner, int32 mipIdx)
{
  if (s.IsReading())
  {
    Data = new FByteBulkData();
  }
  Data->Serialize(s, owner, mipIdx);
  s << SizeX << SizeY;
}