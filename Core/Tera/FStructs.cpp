#include "FStructs.h"
#include "FStream.h"
#include "UObject.h"

#include "Utils/ALog.h"

#include <objbase.h>
#include <NvTriStrip/NvTriStrip.h>

const FMatrix FMatrix::Identity(FPlane(1, 0, 0, 0), FPlane(0, 1, 0, 0), FPlane(0, 0, 1, 0), FPlane(0, 0, 0, 1));
const FVector FVector::One(1, 1, 1);

void VectorMatrixMultiply(void* output, const void* m1, const void* m2)
{
  typedef float Float4x4[4][4];
  const Float4x4& A = *((const Float4x4*)m1);
  const Float4x4& B = *((const Float4x4*)m2);
  Float4x4 tmp;
  tmp[0][0] = A[0][0] * B[0][0] + A[0][1] * B[1][0] + A[0][2] * B[2][0] + A[0][3] * B[3][0];
  tmp[0][1] = A[0][0] * B[0][1] + A[0][1] * B[1][1] + A[0][2] * B[2][1] + A[0][3] * B[3][1];
  tmp[0][2] = A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2] * B[2][2] + A[0][3] * B[3][2];
  tmp[0][3] = A[0][0] * B[0][3] + A[0][1] * B[1][3] + A[0][2] * B[2][3] + A[0][3] * B[3][3];

  tmp[1][0] = A[1][0] * B[0][0] + A[1][1] * B[1][0] + A[1][2] * B[2][0] + A[1][3] * B[3][0];
  tmp[1][1] = A[1][0] * B[0][1] + A[1][1] * B[1][1] + A[1][2] * B[2][1] + A[1][3] * B[3][1];
  tmp[1][2] = A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2] * B[2][2] + A[1][3] * B[3][2];
  tmp[1][3] = A[1][0] * B[0][3] + A[1][1] * B[1][3] + A[1][2] * B[2][3] + A[1][3] * B[3][3];

  tmp[2][0] = A[2][0] * B[0][0] + A[2][1] * B[1][0] + A[2][2] * B[2][0] + A[2][3] * B[3][0];
  tmp[2][1] = A[2][0] * B[0][1] + A[2][1] * B[1][1] + A[2][2] * B[2][1] + A[2][3] * B[3][1];
  tmp[2][2] = A[2][0] * B[0][2] + A[2][1] * B[1][2] + A[2][2] * B[2][2] + A[2][3] * B[3][2];
  tmp[2][3] = A[2][0] * B[0][3] + A[2][1] * B[1][3] + A[2][2] * B[2][3] + A[2][3] * B[3][3];

  tmp[3][0] = A[3][0] * B[0][0] + A[3][1] * B[1][0] + A[3][2] * B[2][0] + A[3][3] * B[3][0];
  tmp[3][1] = A[3][0] * B[0][1] + A[3][1] * B[1][1] + A[3][2] * B[2][1] + A[3][3] * B[3][1];
  tmp[3][2] = A[3][0] * B[0][2] + A[3][1] * B[1][2] + A[3][2] * B[2][2] + A[3][3] * B[3][2];
  tmp[3][3] = A[3][0] * B[0][3] + A[3][1] * B[1][3] + A[3][2] * B[2][3] + A[3][3] * B[3][3];
  memcpy(output, &tmp, 16 * sizeof(float));
}

FGuid FGuid::Generate()
{
  GUID v;
  if (!FAILED(CoCreateGuid(&v)))
  {
    int32* vInt32 = (int32*)&v;
    return FGuid(vInt32[0], vInt32[1], vInt32[2], vInt32[3]);
  }
  return FGuid();
}

FGuid::FGuid(const FString& str)
{
  int cnt = 0;
  uint32 vals[4] = { 0 };
  for (int off = 0; off < str.Size(); off += 1)
  {
    bool skip = false;
    for (int cidx = 0; cidx < 8; ++cidx)
    {
      if (cidx + off >= str.Size())
      {
        LogE("Guid string is malformed or too short: %s", str.C_str());
        return;
      }
      if (!std::isxdigit(str[cidx + off]))
      {
        if (str[cidx + off] == '-')
        {
          skip = true;
          break;
        }
        LogE("Guid item at index %d is not hex %s", cnt, str.Substr(off, 8).C_str());
        return;
      }
    }
    if (skip)
    {
      continue;
    }
    if (cnt > 3)
    {
      LogE("Guid string is malformed: %s", str.C_str());
      return;
    }
    size_t size = 0;
    try
    {
      vals[cnt] = std::stoul(str.Substr(off, 8).WString(), &size, 16);
    }
    catch (...)
    {
      LogE("Failed to parse guid item at index %d - %s", cnt, str.Substr(off, 8).C_str());
      return;
    }
    cnt++;
    off += 7;
  }
  for (int32 idx = 0; idx < 4; ++idx)
  {
    operator[](idx) = vals[idx];
  }
}

FStream& operator<<(FStream& s, FGuid& g)
{
  return s << g.A << g.B << g.C << g.D;
}

FStream& operator<<(FStream& s, FFloat16& v)
{
  return s << v.Packed;
}

FStream& operator<<(FStream& s, FCompressedChunk& c)
{
  return s << c.DecompressedOffset << c.DecompressedSize << c.CompressedOffset << c.CompressedSize;
}

FStream& operator<<(FStream& s, FURL& url)
{
  s << url.Protocol;
  s << url.Host;
  s << url.Map;
  s << url.Portal;
  s << url.Op;
  s << url.Port;
  s << url.Valid;
  return s;
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
  sum.SourceSize = s.GetSize();
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

FStream& operator<<(FStream& s, FVector& v)
{
  return s << v.X << v.Y << v.Z;
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

FStream& operator<<(FStream& s, FBox& b)
{
  s << b.Min;
  s << b.Max;
  s << b.IsValid;
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

FStream& operator<<(FStream& s, FColor& c)
{
  return s << c.R << c.G << c.B << c.A;
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

FStream& operator<<(FStream& s, FBoxSphereBounds& b)
{
  return s << b.Origin << b.BoxExtent << b.SphereRadius;
}

FStream& operator<<(FStream& s, FRotator& r)
{
  return s << r.Pitch << r.Yaw << r.Roll;
}

FStream& operator<<(FStream& s, FQuat& f)
{
  return s << f.X << f.Y << f.Z << f.W;
}

FStream& operator<<(FStream& s, FPackedNormal& n)
{
  return s << n.Vector.Packed;
}

FStream& operator<<(FStream& s, FVector2DHalf& v)
{
  return s << v.X << v.Y;
}

void FPackedPosition::Set(const FVector& inVector)
{
  Vector.X = std::clamp<int32>(trunc(inVector.X * 1023.0f), -1023, 1023);
  Vector.Y = std::clamp<int32>(trunc(inVector.Y * 1023.0f), -1023, 1023);
  Vector.Z = std::clamp<int32>(trunc(inVector.Z * 511.0f), -511, 511);
}

FStream& operator<<(FStream& s, FPackedPosition& p)
{
  return s << p.Packed;
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
        void* tmp = malloc(GetElementSize() * ElementCount);
        MWriteStream memStream(tmp, GetElementSize() * ElementCount);

        // Serialize each element individually via memory reader.				
        for (int32 idx = 0; idx < ElementCount; ++idx)
        {
          SerializeElement(memStream, data, idx);
        }

        s.SerializeCompressed(memStream.GetAllocation(), memStream.GetPosition(), GetDecompressionFlags(), true);
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
    if (BulkDataFlags & BULKDATA_StoreInSeparateFile)
    {
      s << BulkDataSizeOnDisk;
      s << BulkDataOffsetInFile;
    }
    else
    {
      SavedBulkDataFlags = BulkDataFlags;
      SavedElementCount = ElementCount;
      int32 SavedBulkDataSizeOnDiskPos = INDEX_NONE;
      int32 SavedBulkDataOffsetInFilePos = INDEX_NONE;

      SavedBulkDataSizeOnDiskPos = s.GetPosition();
      SavedBulkDataSizeOnDisk = INDEX_NONE;
      s << SavedBulkDataSizeOnDisk;
      SavedBulkDataOffsetInFilePos = s.GetPosition();
      SavedBulkDataOffsetInFile = INDEX_NONE;
      s << SavedBulkDataOffsetInFile;

      int32 SavedBulkDataStartPos = s.GetPosition();
      SerializeBulkData(s, BulkData);
      int32 SavedBulkDataEndPos = s.GetPosition();

      SavedBulkDataSizeOnDisk = SavedBulkDataEndPos - SavedBulkDataStartPos;
      SavedBulkDataOffsetInFile = SavedBulkDataStartPos;


      s.SetPosition(SavedBulkDataSizeOnDiskPos);
      s << SavedBulkDataSizeOnDisk;

      s.SetPosition(SavedBulkDataOffsetInFilePos);
      s << SavedBulkDataOffsetInFile;

      s.SetPosition(SavedBulkDataEndPos);
    }
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
  try
  {
    SerializeBulkData(s, BulkData);
  }
  catch (const std::exception& e)
  {
    LogW("%s", e.what());
    if (s.IsReading())
    {
      free(BulkData);
      BulkData = nullptr;
      OwnsMemory = false;
    }
    if (hasFlag)
    {
      BulkDataFlags |= BULKDATA_StoreInSeparateFile;
    }
    throw e;
  }
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

void FWordBulkData::SerializeElement(FStream& s, void* data, int32 elementIndex)
{
  uint16& d = *((uint16*)data + elementIndex);
  s << d;
}

void FIntBulkData::SerializeElement(FStream& s, void* data, int32 elementIndex)
{
  uint32& d = *((uint32*)data + elementIndex);
  s << d;
}

FPackedNormal::operator FVector() const
{
  float tmp[3] = { (float)Vector.X, (float)Vector.Y, (float)Vector.Z };
  const float a = 1. / 127.5;
  const float b = -1.;
  tmp[0] = tmp[0] * a + b;
  tmp[1] = tmp[1] * a + b;
  tmp[2] = tmp[2] * a + b;
  return FVector(tmp[0], tmp[1], tmp[2]);
}

void FPackedNormal::operator=(const FVector& vec)
{
  Vector.X = std::clamp(Trunc(vec.X * 127.5f + 127.5f), 0, 255);
  Vector.Y = std::clamp(Trunc(vec.Y * 127.5f + 127.5f), 0, 255);
  Vector.Z = std::clamp(Trunc(vec.Z * 127.5f + 127.5f), 0, 255);
  Vector.W = 128;
}

void FPackedNormal::operator=(const FVector4& vec)
{
  Vector.X = std::clamp(Trunc(vec.X * 127.5f + 127.5f), 0, 255);
  Vector.Y = std::clamp(Trunc(vec.Y * 127.5f + 127.5f), 0, 255);
  Vector.Z = std::clamp(Trunc(vec.Z * 127.5f + 127.5f), 0, 255);
  Vector.W = std::clamp(Trunc(vec.W * 127.5f + 127.5f), 0, 255);
}

FStream& operator<<(FStream& s, FMultiSizeIndexContainer& c)
{
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << c.NeedsCPUAccess;
  }
  s << c.ElementSize;
  s << c.BulkElementSize;

  if (c.ElementSize != c.BulkElementSize)
  {
    UThrow("Elements size mismatch: %u != %u", c.ElementSize, c.BulkElementSize);
  }

  s << c.ElementCount;
  if (s.IsReading())
  {
    c.AllocateBuffer(c.ElementCount, c.ElementSize);
  }
  s.SerializeBytes(c.IndexBuffer, c.ElementSize * c.ElementCount);
  return s;
}

FStream& operator<<(FStream& s, FRawIndexBuffer& b)
{
  s << b.ElementSize;
  s << b.ElementCount;
  if (s.IsReading())
  {
    b.AllocateBuffer(b.ElementCount, b.ElementSize);
  }
  s.SerializeBytes(b.Data, b.ElementSize * b.ElementCount);
  return s;
}

FStream& operator<<(FStream& s, FSphere& sp)
{
  return s << sp.Center << sp.Radius;
}

FStream& operator<<(FStream& s, FStreamableTextureInstance& i)
{
  return s << i.BoundingSphere << i.TexelFactor;
}

FStream& operator<<(FStream& s, FDynamicTextureInstance& i)
{
  s << i.BoundingSphere;
  s << i.TexelFactor;
  s << i.Texture;
  s << i.bAttached;
  s << i.OriginalRadius;
  return s;
}

FStream& operator<<(FStream& s, FCachedPhysSMData& d)
{
  return s << d.Scale3D << d.CachedDataIndex;
}

FStream& operator<<(FStream& s, FCachedPerTriPhysSMData& d)
{
  return s << d.Scale3D << d.CachedDataIndex;
}

FStream& operator<<(FStream& s, FKCachedPerTriData& d)
{
  s << d.CachedPerTriDataElementSize;
  s << d.CachedPerTriData;
  return s;
}

FStream& operator<<(FStream& s, FVolumeLightingSample& ls)
{
  s << ls.Position;
  s << ls.Radius;
  s << ls.IndirectDirectionTheta;
  s << ls.IndirectDirectionPhi;
  s << ls.EnvironmentDirectionTheta;
  s << ls.EnvironmentDirectionPhi;
  s << ls.IndirectRadiance;
  s << ls.EnvironmentRadiance;
  s << ls.AmbientRadiance;
  s << ls.bShadowedFromDominantLights;
  return s;
}

FStream& operator<<(FStream& s, FPrecomputedLightVolume& v)
{
  s << v.bInitialized;
  if (v.bInitialized)
  {
    s << v.Bounds;
    s << v.SampleSpacing;
    s << v.Samples;
  }
  return s;
}

FStream& operator<<(FStream& s, FPrecomputedVisibilityCell& c)
{
  return s << c.Min << c.ChunkIndex << c.DataOffset;
}

FStream& operator<<(FStream& s, FCompressedVisibilityChunk& c)
{
  return s << c.bCompressed << c.UncompressedSize << c.Data;
}

FStream& operator<<(FStream& s, FPrecomputedVisibilityBucket& b)
{
  return s << b.CellDataSize << b.Cells << b.CellDataChunks;
}

FStream& operator<<(FStream& s, FPrecomputedVisibilityHandler& h)
{
  s << h.BucketOriginXY;
  s << h.SizeXY;
  s << h.SizeZ;
  s << h.BucketSizeXY;
  s << h.NumBuckets;
  s << h.Buckets;
  return s;
}

FStream& operator<<(FStream& s, FPrecomputedVolumeDistanceField& f)
{
  s << f.VolumeMaxDistance;
  s << f.VolumeBox;
  s << f.VolumeSizeX;
  s << f.VolumeSizeY;
  s << f.VolumeSizeZ;
  s << f.Data;
  return s;
}

FRotator FRotator::Normalized() const
{
  FRotator result;
  result.Pitch = NormalizeAxis(Pitch);
  result.Roll = NormalizeAxis(Roll);
  result.Yaw = NormalizeAxis(Yaw);
  return result;
}

FRotator FRotator::Denormalized() const
{
  FRotator result(*this);
  result.Pitch = result.Pitch & 0xFFFF;
  result.Yaw = result.Yaw & 0xFFFF;
  result.Roll = result.Roll & 0xFFFF;
  return result;
}

FVector FRotator::Euler() const
{
  return FVector(Roll * (180.f / 32768.f), Pitch * (180.f / 32768.f), Yaw * (180.f / 32768.f));
}

FQuat FRotator::Quaternion() const
{
  return FQuat(FRotationMatrix(*this));
}

FRotator FRotator::GetInverse() const
{
  return Quaternion().Inverse().Rotator();
}

FMatrix::FMatrix(const FPlane& x, const FPlane& y, const FPlane& z, const FPlane& w)
{
  M[0][0] = x.X; M[0][1] = x.Y;  M[0][2] = x.Z;  M[0][3] = x.W;
  M[1][0] = y.X; M[1][1] = y.Y;  M[1][2] = y.Z;  M[1][3] = y.W;
  M[2][0] = z.X; M[2][1] = z.Y;  M[2][2] = z.Z;  M[2][3] = z.W;
  M[3][0] = w.X; M[3][1] = w.Y;  M[3][2] = w.Z;  M[3][3] = w.W;
}

void FMatrix::operator*=(const FMatrix& b)
{
  VectorMatrixMultiply(&M, &M, &b.M);
}

FMatrix FMatrix::operator*(const FMatrix& a) const
{
  FMatrix Result;
  VectorMatrixMultiply(&Result, this, &a);
  return Result;
}

FVector FMatrix::Rotate(FVector& v)
{
  float d[] = { v.X, v.Y, v.Z, 0 };
  float dx = M[0][0] * d[0] + M[1][0] * d[1] + M[2][0] * d[2] + M[3][0] * d[3];

  float dy = M[0][1] * d[0] + M[1][1] * d[1] + M[2][1] * d[2] + M[3][1] * d[3];
  float dz = M[0][2] * d[0] + M[0][2] * d[1] + M[2][2] * d[2] + M[3][2] * d[3];

  if (abs(dx) < 0.00001)
  {
    dx = 0;
  }
  if (abs(dy) < 0.00001)
  {
    dy = 0;
  }
  if (abs(dz) < 0.00001)
  {
    dz = 0;
  }
  return FVector(dx, dy, dz);
}

FQuat::FQuat(const FMatrix& matrix)
{
  const float tr = matrix.M[0][0] + matrix.M[1][1] + matrix.M[2][2];
  if (tr > 0.0f)
  {
    float invS = 1.f / sqrtf(tr + 1.f);
    W = 0.5f * (1.f / invS);
    float s = 0.5f * invS;
    X = (matrix.M[1][2] - matrix.M[2][1]) * s;
    Y = (matrix.M[2][0] - matrix.M[0][2]) * s;
    Z = (matrix.M[0][1] - matrix.M[1][0]) * s;
  }
  else
  {
    int32 i = 0;
    if (matrix.M[1][1] > matrix.M[0][0])
    {
      i = 1;
    }
    if (matrix.M[2][2] > matrix.M[i][i])
    {
      i = 2;
    }

    static const int32 nxt[3] = { 1, 2, 0 };
    const int32 j = nxt[i];
    const int32 k = nxt[j];

    float s = matrix.M[i][i] - matrix.M[j][j] - matrix.M[k][k] + 1.0f;

    float invS = 1.f / sqrtf(s);

    float qt[4];
    qt[i] = 0.5f * (1.f / invS);

    s = 0.5f * invS;

    qt[3] = (matrix.M[j][k] - matrix.M[k][j]) * s;
    qt[j] = (matrix.M[i][j] + matrix.M[j][i]) * s;
    qt[k] = (matrix.M[i][k] + matrix.M[k][i]) * s;

    X = qt[0];
    Y = qt[1];
    Z = qt[2];
    W = qt[3];
  }
}

FQuat FQuat::Inverse() const
{
  return FQuat(-X, -Y, -Z, W);
}

FRotator FQuat::Rotator() const
{
  // UE4
  const float singularityTest = Z * X - W * Y;
  const float yawY = 2.f * (W * Z + X * Y);
  const float yawX = (1.f - 2.f * ((Y * Y) + (Z * Z)));

  const float SINGULARITY_THRESHOLD = 0.4999995f;
  const float RAD_TO_DEG = (180.f) / M_PI;
  FVector tmp;

  if (singularityTest < -SINGULARITY_THRESHOLD)
  {
    tmp.Y = -90.f;
    tmp.Z = atan2(yawY, yawX) * RAD_TO_DEG;
    tmp.X = FRotator::NormalizeAxis(-tmp.Z - (2.f * atan2f(X, W) * RAD_TO_DEG));
  }
  else if (singularityTest > SINGULARITY_THRESHOLD)
  {
    tmp.Y = 90.f;
    tmp.Z = atan2f(yawY, yawX) * RAD_TO_DEG;
    tmp.X = FRotator::NormalizeAxis(tmp.Z - (2.f * atan2f(X, W) * RAD_TO_DEG));
  }
  else
  {
    tmp.Y = asinf(2.f * (singularityTest)) * RAD_TO_DEG;
    tmp.Z = atan2f(yawY, yawX) * RAD_TO_DEG;
    tmp.X = atan2f(-2.f * (W * X + Y * Z), (1.f - 2.f * ((X * X) + (Y * Y)))) * RAD_TO_DEG;
  }
  return FRotator::MakeFromEuler(FVector(tmp.Y, tmp.Z, tmp.X));
}

FRotationTranslationMatrix::FRotationTranslationMatrix(const FRotator& rotation, const FVector& origin)
{
  const float	sr = (float)sin((double)rotation.Roll * (M_PI / 32768.));
  const float	cr = (float)cos((double)rotation.Roll * (M_PI / 32768.));
  const float	sp = (float)sin((double)rotation.Pitch * (M_PI / 32768.));
  const float	cp = (float)cos((double)rotation.Pitch * (M_PI / 32768.));
  const float	sy = (float)sin((double)rotation.Yaw * (M_PI / 32768.));
  const float	cy = (float)cos((double)rotation.Yaw * (M_PI / 32768.));

  M[0][0] = cp * cy;
  M[0][1] = cp * sy;
  M[0][2] = sp;
  M[0][3] = 0.f;

  M[1][0] = sr * sp * cy - cr * sy;
  M[1][1] = sr * sp * sy + cr * cy;
  M[1][2] = -sr * cp;
  M[1][3] = 0.f;

  M[2][0] = -(cr * sp * cy + sr * sy);
  M[2][1] = cy * sr - cr * sp * sy;
  M[2][2] = cr * cp;
  M[2][3] = 0.f;

  M[3][0] = origin.X;
  M[3][1] = origin.Y;
  M[3][2] = origin.Z;
  M[3][3] = 1.f;
}

FVector::FVector(const FVector4& v)
  : X(v.X)
  , Y(v.Y)
  , Z(v.Z)
{}

FColor FLinearColor::ToFColor(bool sRGB) const
{
  float r = std::clamp(R, 0.0f, 1.0f);
  float g = std::clamp(G, 0.0f, 1.0f);
  float b = std::clamp(B, 0.0f, 1.0f);
  float a = std::clamp(A, 0.0f, 1.0f);

  if (sRGB)
  {
    r = powf(r, 1.0f / 2.2f);
    g = powf(g, 1.0f / 2.2f);
    b = powf(b, 1.0f / 2.2f);
  }

  FColor result;

  result.A = floorf(a * 255.999f);
  result.R = floorf(r * 255.999f);
  result.G = floorf(g * 255.999f);
  result.B = floorf(b * 255.999f);

  return result;
}

void FRawIndexBuffer::SortIndices()
{
  PrimitiveGroup* PrimitiveGroups = nullptr;
  uint32 NumPrimitiveGroups = 0;
  bool isInt = ElementSize == sizeof(uint32);
  SetListsOnly(true);
  if (!isInt)
  {
    void* tmpData = malloc(sizeof(uint32) * ElementCount);
    for (uint32 idx = 0; idx < ElementCount; ++idx)
    {
      ((uint32*)tmpData)[idx] = GetIndex(idx);
    }
    GenerateStrips((const unsigned int*)tmpData, ElementCount, &PrimitiveGroups, &NumPrimitiveGroups);
    free(tmpData);
  }
  else
  {
    GenerateStrips((const unsigned int*)Data, ElementCount, &PrimitiveGroups, &NumPrimitiveGroups);
  }
  if (PrimitiveGroups)
  {
    free(Data);
    Data = nullptr;
    ElementCount = PrimitiveGroups->numIndices;
    AllocateBuffer(ElementCount, ElementSize);
    if (isInt)
    {
      std::memcpy(Data, PrimitiveGroups->indices, ElementCount * ElementSize);
    }
    else
    {
      for (uint32 idx = 0; idx < PrimitiveGroups->numIndices; ++idx)
      {
        SetIndex(idx, (uint16)PrimitiveGroups->indices[idx]);
      }
    }
    delete[] PrimitiveGroups;
  }
}