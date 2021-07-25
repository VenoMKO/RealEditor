#pragma once
#include "Core.h"
#include "FString.h"
#include "FName.h"

#define Quant16BitDiv     (32767.f)
#define Quant16BitFactor  (32767.f)
#define Quant16BitOffs    (32767)

#define Quant10BitDiv     (511.f)
#define Quant10BitFactor  (511.f)
#define Quant10BitOffs    (511)

#define Quant11BitDiv     (1023.f)
#define Quant11BitFactor  (1023.f)
#define Quant11BitOffs    (1023)

struct FGuid
{
public:

  static FGuid Generate();

  FGuid() = default;

  // Accepts formats: x-x-x-x, x-x-xx, etc...
  // If failed to parse the GUID will be invalid(0000)
  FGuid(const FString& str);

  FGuid(uint32 a, uint32 b, uint32 c, uint32 d)
    : A(a)
    , B(b)
    , C(c)
    , D(d)
  {}

  bool IsValid() const
  {
    return (A | B | C | D) != 0;
  }

  void Invalidate()
  {
    A = B = C = D = 0;
  }

  friend bool operator==(const FGuid& x, const FGuid& y)
  {
    return ((x.A ^ y.A) | (x.B ^ y.B) | (x.C ^ y.C) | (x.D ^ y.D)) == 0;
  }

  friend bool operator!=(const FGuid& x, const FGuid& y)
  {
    return ((x.A ^ y.A) | (x.B ^ y.B) | (x.C ^ y.C) | (x.D ^ y.D)) != 0;
  }

  uint32& operator[](int32 Index)
  {
    switch (Index)
    {
    case 0: return A;
    case 1: return B;
    case 2: return C;
    case 3: return D;
    }
    return A;
  }

  const uint32& operator[](int32 Index) const
  {
    switch (Index)
    {
    case 0: return A;
    case 1: return B;
    case 2: return C;
    case 3: return D;
    }

    return A;
  }

  bool operator<(const FGuid& b) const
  {
    return String() < b.String();
  }

  friend FStream& operator<<(FStream& s, FGuid& g);

  FString String() const
  {
    return FString::Sprintf("%08X%08X%08X%08X", A, B, C, D);
  }

  FString FormattedString() const
  {
    return FString::Sprintf("%08X-%08X-%08X-%08X", A, B, C, D);
  }

private:
  uint32 A = 0;
  uint32 B = 0;
  uint32 C = 0;
  uint32 D = 0;
};

struct FCompressedChunk {
  FILE_OFFSET DecompressedOffset = 0;
  FILE_OFFSET DecompressedSize = 0;
  FILE_OFFSET CompressedOffset = 0;
  FILE_OFFSET CompressedSize = 0;

  friend FStream& operator<<(FStream& s, FCompressedChunk& c);
};

struct FURL {
  FString Protocol;
  FString Host;
  int32 Port = 0;
  FString Map;
  FString Op;
  FString Portal;
  int32 Valid = 0;

  friend FStream& operator<<(FStream& s, FURL& url);
};

struct FGeneration {
  int32 Exports = 0;
  int32 Names = 0;
  int32 NetObjects = 0;

  friend FStream& operator<<(FStream& s, FGeneration& g);
};

struct FTextureAllocations {
  struct FTextureType
  {
    int32 SizeX = 0;
    int32 SizeY = 0;
    int32 NumMips = 0;
    uint32 Format = 0;
    uint32 TexCreateFlags = 0; //ETextureCreateFlags
    std::vector<int32> ExportIndices;

    friend FStream& operator<<(FStream& s, FTextureAllocations::FTextureType& t);
  };

  std::vector<FTextureType>	TextureTypes;

  friend FStream& operator<<(FStream& s, FTextureAllocations& t);
};

struct FCompositePackageMapEntry {
  FString ObjectPath;
  FString FileName;
  FILE_OFFSET Offset = 0;
  FILE_OFFSET Size = 0;

  friend FStream& operator<<(FStream& s, FCompositePackageMapEntry& e);
};

struct FCompressedChunkInfo {
  int32 CompressedSize = 0;
  int32 DecompressedSize = 0;
  int32 DecompressedOffset = 0;

  friend FStream& operator<<(FStream& s, FCompressedChunkInfo& c);
};

struct FPackageSummary {
  uint32 Magic = PACKAGE_MAGIC;
  uint32 FileVersion = 0;
  int32 HeaderSize = 0;
  FString FolderName = "None";
  uint32 PackageFlags = 0;
  uint32 OriginalPackageFlags = 0;
  uint32 NamesCount = 0;
  FILE_OFFSET NamesOffset = 0;
  FILE_OFFSET NamesSize = 0;
  uint32 ExportsCount = 0;
  FILE_OFFSET ExportsOffset = 0;
  uint32 ImportsCount = 0;
  FILE_OFFSET ImportsOffset = 0;
  FILE_OFFSET DependsOffset = 0;
  FILE_OFFSET ImportExportGuidsOffset = 0;
  uint32 ImportGuidsCount = 0;
  uint32 ExportGuidsCount = 0;
  uint32 ThumbnailTableOffset = 0;
  FGuid Guid;
  std::vector<FGeneration> Generations;
  int32 EngineVersion = 4206;
  int32 ContentVersion = 76;
  uint32 CompressionFlags = 0;
  uint32 OriginalCompressionFlags = 0;
  uint32 PackageSource = 0;
  std::vector<FCompressedChunk> CompressedChunks;
  std::vector<FString> AdditionalPackagesToCook;
  FTextureAllocations TextureAllocations;

  size_t SourceSize = 0;

  uint16 GetFileVersion() const
  {
    return (FileVersion & 0xFFFF);
  }

  uint16 GetLicenseeVersion() const
  {
    return ((FileVersion >> 16) & 0xFFFF);
  }

  void SetFileVersion(uint16 engine, uint16 licensee)
  {
    FileVersion = ((licensee << 16) | engine);;
  }
  // Path to the package
  FString SourcePath;
  // Path to the decompressed package. Equals to the SourcePath if a package is not compressed
  FString DataPath;

  FString PackageName = "Untitled.gpk";

  friend FStream& operator<<(FStream& s, FPackageSummary& sum);
};

struct FMeshEdge
{
  int32	Vertices[2];
  int32	Faces[2];

  friend FStream& operator<<(FStream& s, FMeshEdge& e);
};

struct FVector2D {
  FVector2D()
  {}

  FVector2D(float a)
    : X(a)
    , Y(a)
  {}

  FVector2D(float x, float y)
    : X(x)
    , Y(y)
  {}

  bool operator==(const FVector2D& v) const
  {
    return X == v.X && Y == v.Y;
  }

  float X = 0;
  float Y = 0;

  friend FStream& operator<<(FStream& s, FVector2D& v);
};

struct FVector4;
struct FVector {
  static const FVector One;
  static const FVector Zero;

  FVector()
  {}

  FVector(const FVector& v)
    : X(v.X)
    , Y(v.Y)
    , Z(v.Z)
  {}

  FVector(const FVector4& v);

  FVector(float a)
    : X(a)
    , Y(a)
    , Z(a)
  {}

  FVector(float x, float y, float z)
    : X(x)
    , Y(y)
    , Z(z)
  {}

  bool IsZero() const
  {
    return !X && !Y && !Z;
  }

  bool operator==(const FVector& v) const
  {
    return X == v.X && Y == v.Y && Z == v.Z;
  }

  bool operator!=(const FVector& v) const
  {
    return X != v.X || Y != v.Y || Z != v.Z;
  }

  // Dot product
  float operator|(const FVector& v) const
  {
    return X * v.X + Y * v.Y + Z * v.Z;
  }

  // Cross product
  FVector operator^(const FVector& v) const
  {
    return FVector
    (
      Y * v.Z - Z * v.Y,
      Z * v.X - X * v.Z,
      X * v.Y - Y * v.X
    );
  }

  FVector operator*(float s) const
  {
    return FVector(X * s, Y * s, Z * s);
  }

  FVector& operator-=(FVector v)
  {
    X -= v.X;
    Y -= v.Y;
    Z -= v.Z;
    return *this;
  }

  FVector operator-(const FVector& v) const
  {
    return FVector(X - v.X, Y - v.Y, Z - v.Z);
  }

  FVector operator-() const
  {
    return FVector(-X, -Y, -Z);
  }

  FVector& operator*=(float s)
  {
    X *= s;
    Y *= s;
    Z *= s;
    return *this;
  }

  FVector operator*(const FVector& v) const
  {
    return FVector(X * v.X, Y * v.Y, Z * v.Z);
  }

  FVector& operator*=(const FVector& v)
  {
    X *= v.X;
    Y *= v.Y;
    Z *= v.Z;
    return *this;
  }

  FVector operator/(float v) const
  {
    return FVector(X / v, Y / v, Z / v);
  }

  FVector operator+(const FVector& v) const
  {
    return FVector(X + v.X, Y + v.Y, Z + v.Z);
  }

  FVector& operator+=(const FVector& v)
  {
    X += v.X; Y += v.Y; Z += v.Z;
    return *this;
  }

  float& operator[](const int32 idx)
  {
    switch (idx)
    {
    case 0:
      return X;
    case 1:
      return Y;
    }
    return Z;
  }

  const float& operator[](const int32 idx) const
  {
    switch (idx)
    {
    case 0:
      return X;
    case 1:
      return Y;
    }
    return Z;
  }

  bool Normalize()
  {
    const float ss = X * X + Y * Y + Z * Z;
    if (ss > 1.e-8)
    {
      const float s = 1.f / sqrtf(ss);
      X *= s; Y *= s; Z *= s;
      return true;
    }
    return false;
  }

  FVector SafeNormal(float tolerance = 1.e-8) const
  {
    float ss = X * X + Y * Y + Z * Z;

    if (ss == 1.f)
    {
      return *this;
    }
    if (ss < tolerance)
    {
      return FVector(0.f);
    }
    float s = 1.f / sqrtf(ss);
    return FVector(X * s, Y * s, Z * s);
  }

  float Size() const
  {
    return sqrt(X * X + Y * Y + Z * Z);
  }

  friend FStream& operator<<(FStream& s, FVector& v);

  float X = 0;
  float Y = 0;
  float Z = 0;
};

struct FVector4 {
  FVector4(float x = 0., float y = 0., float z = 0., float w = 0.)
    : X(x)
    , Y(y)
    , Z(z)
    , W(w)
  {}

  FVector4 SafeNormal(float tolerance = 1.e-8) const
  {
    const float ss = X * X + Y * Y + Z * Z;
    if (ss > tolerance)
    {
      const float s = 1.f / sqrtf(ss);
      return FVector4(X * s, Y * s, Z * s, 0.0f);
    }
    return FVector4(0.f);
  }

  float X = 0;
  float Y = 0;
  float Z = 0;
  float W = 0;
};

struct FScriptDelegate
{
  DECL_UREF(UObject, Object);
  FName FunctionName;

  inline FString ToString(const UObject* OwnerObject) const;
  friend FStream& operator<<(FStream& s, FScriptDelegate& d);
};

struct FPacakgeTreeEntry
{
  FStringRef FullObjectName;
  FStringRef ClassName;

  friend FStream& operator<<(FStream& s, FPacakgeTreeEntry& e);
};

struct FCookedBulkDataInfo
{
  uint32 SavedBulkDataFlags = 0;
  uint32 SavedElementCount = 0;
  uint32 SavedBulkDataOffsetInFile = 0;
  uint32 SavedBulkDataSizeOnDisk = 0;
  FName	TextureFileCacheName;

  friend FStream& operator<<(FStream& s, FCookedBulkDataInfo& i);
};

struct FBulkDataInfo
{
  FBulkDataInfo() = default;

  FBulkDataInfo(const FCookedBulkDataInfo& v)
    : SavedBulkDataFlags(v.SavedBulkDataFlags)
    , SavedElementCount(v.SavedElementCount)
    , SavedBulkDataOffsetInFile(v.SavedBulkDataOffsetInFile)
    , SavedBulkDataSizeOnDisk(v.SavedBulkDataSizeOnDisk)
    , TextureFileCacheName(v.TextureFileCacheName.String())
  {}

  uint32 SavedBulkDataFlags = 0;
  uint32 SavedElementCount = 0;
  uint32 SavedBulkDataOffsetInFile = 0;
  uint32 SavedBulkDataSizeOnDisk = 0;
  FString	TextureFileCacheName;
};

struct FBox {
  FVector Min;
  FVector Max;
  uint8 IsValid = 0;

  FBox() = default;

  FBox(const std::vector<FVector>& points);

  FBox& operator+=(const FVector& v)
  {
    if (IsValid)
    {
      Min.X = std::min(Min.X, v.X);
      Min.Y = std::min(Min.Y, v.Y);
      Min.Z = std::min(Min.Z, v.Z);

      Max.X = std::max(Max.X, v.X);
      Max.Y = std::max(Max.Y, v.Y);
      Max.Z = std::max(Max.Z, v.Z);
    }
    else
    {
      Min = Max = v;
      IsValid = 1;
    }
    return *this;
  }

  FVector GetExtent() const
  {
    return (Max - Min) * 0.5f;
  }

  void GetCenterAndExtents(FVector& center, FVector& extents) const
  {
    extents = GetExtent();
    center = Min + extents;
  }

  friend FStream& operator<<(FStream& s, FBox& b);
};

struct FCookedTextureFileCacheInfo
{
  FGuid TextureFileCacheGuid;
  FName TextureFileCacheName;
  double LastSaved = 0;

  friend FStream& operator<<(FStream& s, FCookedTextureFileCacheInfo& i);
};

struct FTextureFileCacheInfo
{
  FTextureFileCacheInfo()
  {}

  FTextureFileCacheInfo(const FCookedTextureFileCacheInfo& i)
    : TextureFileCacheGuid(i.TextureFileCacheGuid)
    , TextureFileCacheName(i.TextureFileCacheName.String())
    , LastSaved(i.LastSaved)
  {}

  FGuid TextureFileCacheGuid;
  FString TextureFileCacheName;
  double LastSaved = 0;
};

struct FCookedTextureUsageInfo
{
  std::vector<FStringRef>	PackageNames;
  uint8 Format = 0xFF;
  uint8 LODGroup = 0xFF;
  int32 SizeX = 0;
  int32 SizeY = 0;
  int32 StoredOnceMipSize = 0;
  int32 DuplicatedMipSize = 0;

  friend FStream& operator<<(FStream& s, FCookedTextureUsageInfo& i);
};

struct FForceCookedInfo
{
  std::map<FString, bool> CookedContentList;

  friend FStream& operator<<(FStream& s, FForceCookedInfo& i);
};

struct AMetaDataEntry {
  FString Name;
  FString Tooltip;

  friend FStream& operator<<(FStream& s, AMetaDataEntry& e);
};

struct FSHA
{
  uint8* GetBytes()
  {
    return Bytes;
  }

  const uint8* GetBytes() const
  {
    return Bytes;
  }

  friend FStream& operator<<(FStream& s, FSHA& i);

protected:
  uint8 Bytes[20];
};

struct FUntypedBulkData
{
  FUntypedBulkData() = default;

  FUntypedBulkData(const FUntypedBulkData& a)
  {
    BulkDataFlags = a.BulkDataFlags;
    ElementCount = a.ElementCount;
    BulkDataOffsetInFile = a.BulkDataOffsetInFile;
    BulkDataSizeOnDisk = a.BulkDataSizeOnDisk;

    SavedBulkDataFlags = a.SavedBulkDataFlags;
    SavedElementCount = a.SavedElementCount;
    SavedBulkDataOffsetInFile = a.SavedBulkDataOffsetInFile;
    SavedBulkDataSizeOnDisk = a.SavedBulkDataSizeOnDisk;

    if (OwnsMemory && BulkData)
    {
      free(BulkData);
    }
    
    OwnsMemory = a.OwnsMemory;
    Package = a.Package;
    if (ElementCount)
    {
      if (OwnsMemory)
      {
        BulkData = malloc(a.GetElementSize() * ElementCount);
        std::memcpy(BulkData, a.BulkData, a.GetElementSize() * ElementCount);
      }
      else
      {
        BulkData = a.BulkData;
      }
    }
  }

  FUntypedBulkData& operator=(const FUntypedBulkData& a)
  {
    BulkDataFlags = a.BulkDataFlags;
    ElementCount = a.ElementCount;
    BulkDataOffsetInFile = a.BulkDataOffsetInFile;
    BulkDataSizeOnDisk = a.BulkDataSizeOnDisk;

    SavedBulkDataFlags = a.SavedBulkDataFlags;
    SavedElementCount = a.SavedElementCount;
    SavedBulkDataOffsetInFile = a.SavedBulkDataOffsetInFile;
    SavedBulkDataSizeOnDisk = a.SavedBulkDataSizeOnDisk;

    if (OwnsMemory && BulkData)
    {
      free(BulkData);
    }

    OwnsMemory = a.OwnsMemory;
    Package = a.Package;
    if (ElementCount)
    {
      if (OwnsMemory)
      {
        BulkData = malloc(a.GetElementSize() * ElementCount);
        std::memcpy(BulkData, a.BulkData, a.GetElementSize() * ElementCount);
      }
      else
      {
        BulkData = a.BulkData;
      }
    }
    return *this;
  }

  FUntypedBulkData(FPackage* package, uint32 bulkDataFlags, int32 elementCount, void* bulkData, bool transferOwnership)
  {
    BulkDataFlags = bulkDataFlags;
    ElementCount = elementCount;
    BulkData = bulkData;
    OwnsMemory = transferOwnership;
    Package = package;
  }

  virtual ~FUntypedBulkData();

  void* GetAllocation()
  {
    return BulkData;
  }

  const void* GetAllocation() const
  {
    return BulkData;
  }

  void Realloc(int32 elementCount)
  {
    if (elementCount <= 0)
    {
      return;
    }
    if (OwnsMemory)
    {
      free(BulkData);
    }
    OwnsMemory = true;
    BulkData = malloc(elementCount * GetElementSize());
    ElementCount = elementCount;
  }

  int32 GetElementCount() const;

  int32 GetBulkDataSize() const;

  int32 GetBulkDataSizeOnDisk() const;

  int32 GetBulkDataOffsetInFile() const;

  bool IsStoredCompressedOnDisk() const;

  bool IsStoredInSeparateFile() const;

  ECompressionFlags GetDecompressionFlags() const;

  void Serialize(FStream& s, UObject* owner, int32 idx = INDEX_NONE);

  void SerializeSeparate(FStream& s, UObject* owner, int32 idx = INDEX_NONE);

  void SerializeBulkData(FStream& s, void* data);

  virtual bool RequiresSingleElementSerialization(FStream& s);

  void GetCopy(void** dest) const;

  virtual int32 GetElementSize() const = 0;

  virtual void SerializeElement(FStream& s, void* data, int32 elementIndex) = 0;

  uint32 BulkDataFlags = BULKDATA_None;
  int32 ElementCount = 0;
  int32 BulkDataOffsetInFile = INDEX_NONE;
  int32 BulkDataSizeOnDisk = INDEX_NONE;

  uint32 SavedBulkDataFlags = BULKDATA_None;
  int32 SavedElementCount = 0;
  int32 SavedBulkDataOffsetInFile = INDEX_NONE;
  int32 SavedBulkDataSizeOnDisk = INDEX_NONE;

  bool OwnsMemory = false;
  void* BulkData = nullptr;
  FPackage* Package = nullptr;
};

struct FByteBulkData : public FUntypedBulkData
{
  using FUntypedBulkData::FUntypedBulkData;

  int32 GetElementSize() const override;
  void SerializeElement(FStream& s, void* data, int32 elementIndex) override;
};

struct FIntPoint {
  int32 X = 0;
  int32 Y = 0;

  friend FStream& operator<<(FStream& s, FIntPoint& p);
};

class FColor {
public:
  FColor()
  {}

  FColor(uint8 r, uint8 g, uint8 b, uint8 a = 0)
    : B(b)
    , G(g)
    , R(r)
    , A(a)
  {}

  FColor(uint32 dwColor)
  {
    union {
      struct {
        uint8 TB;
        uint8 TG;
        uint8 TR;
        uint8 TA;
      };
      uint32 DW = 0;
    };
    DW = dwColor;
    B = TB;
    G = TG;
    R = TR;
    A = TA;
  }

  uint32 DWColor() const
  {
    return (*(uint32*)this);
  }

  bool operator==(const FColor& v) const
  {
    return R == v.R && G == v.G && B == v.B && A == v.A;
  }

  friend FStream& operator<<(FStream& s, FColor& c);

  uint8 B = 0;
  uint8 G = 0;
  uint8 R = 0;
  uint8 A = 0;
};

class FLinearColor {
public:
  FLinearColor()
  {}

  FLinearColor(float r, float g, float b, float a)
    : R(r)
    , G(g)
    , B(b)
    , A(a)
  {}

  FLinearColor(FColor& c)
    : R(powf(c.R / 255.f, 2.2f))
    , G(powf(c.G / 255.f, 2.2f))
    , B(powf(c.B / 255.f, 2.2f))
    , A(powf(c.A / 255.f, 2.2f))
  {}

  bool operator==(const FLinearColor& v) const
  {
    return R == v.R && G == v.G && B == v.B && A == v.A;
  }

  bool operator!=(const FLinearColor& v) const
  {
    return R != v.R || G != v.G || B != v.B || A != v.A;
  }

  friend FStream& operator<<(FStream& s, FLinearColor& c);

  FLinearColor operator*(float s) const
  {
    return FLinearColor(
      this->R * s,
      this->G * s,
      this->B * s,
      this->A * s
    );
  }

  FLinearColor& operator*=(float s)
  {
    R *= s;
    G *= s;
    B *= s;
    A *= s;
    return *this;
  }

  FColor ToFColor(bool sRGB) const;

  float R = 0;
  float G = 0;
  float B = 0;
  float A = 0;
};

struct FTexture2DMipMap
{
  ~FTexture2DMipMap();

  void Serialize(FStream& s, UObject* owner, int32 mipIdx);

  FByteBulkData* Data = nullptr;
  int32 SizeX = 0;
  int32 SizeY = 0;
};

struct FIntRect
{
  FIntPoint Min;
  FIntPoint Max;

  friend FStream& operator<<(FStream& s, FIntRect& r);
};

struct FLevelGuids
{
  friend FStream& operator<<(FStream& s, FLevelGuids& g);

  FName LevelName;
  std::vector<FGuid> Guids;
};

struct FObjectThumbnailInfo {

  friend FStream& operator<<(FStream& s, FObjectThumbnailInfo& i);

  FString ObjectClassName;
  FString ObjectPath;
  int32 Offset = 0;
};

struct FObjectThumbnail {

  ~FObjectThumbnail()
  {
    free(CompressedData);
  }

  friend FStream& operator<<(FStream& s, FObjectThumbnail& t);

  int32 Width = 0;
  int32 Height = 0;

  int32 CompressedSize = 0;
  void* CompressedData = nullptr;
  bool IsDirty = false;
};

struct FBoxSphereBounds {
  FVector	Origin;
  FVector	BoxExtent;
  float	SphereRadius = 0;

  FBoxSphereBounds() = default;

  FBoxSphereBounds(const FBox& box)
  {
    box.GetCenterAndExtents(Origin, BoxExtent);
    SphereRadius = BoxExtent.Size();
  }

  FBoxSphereBounds operator+(const FBoxSphereBounds& a) const
  {
    FBox	box;
    box += (Origin - BoxExtent);
    box += (Origin + BoxExtent);
    box += (a.Origin - a.BoxExtent);
    box += (a.Origin + a.BoxExtent);

    FBoxSphereBounds	result(box);
    result.SphereRadius = std::min(
      result.SphereRadius,
      std::max(
        (Origin - result.Origin).Size() + SphereRadius,
        (a.Origin - result.Origin).Size() + a.SphereRadius
      )
    );

    return result;
  }

  friend FStream& operator<<(FStream& s, FBoxSphereBounds& bounds);
};

struct FPlane : FVector {
  FPlane() = default;

  FPlane(float x, float y, float z, float w)
    : FVector(x, y, z)
    , W(w)
  {}

  FPlane(const FVector& a, float w)
    : FVector(a)
    , W(w)
  {}

  FPlane(FVector A, FVector B, FVector C)
    : FVector(((B - A) ^ (C - A)).SafeNormal())
    , W(A | ((B - A) ^ (C - A)).SafeNormal())
  {}

  float W = 0;
};

struct FMatrix {
  static const FMatrix Identity;
  FMatrix() = default;
  FMatrix(const FPlane& x, const FPlane& y, const FPlane& z, const FPlane& w);

  inline FVector GetAxis(int32 i) const
  {
    return FVector(M[i][0], M[i][1], M[i][2]);
  }

  inline FVector4 TransformFVector4(const FVector4& P) const
  {
    FVector4 Result;
    Result.X = P.X * M[0][0] + P.Y * M[1][0] + P.Z * M[2][0] + P.W * M[3][0];
    Result.Y = P.X * M[0][1] + P.Y * M[1][1] + P.Z * M[2][1] + P.W * M[3][1];
    Result.Z = P.X * M[0][2] + P.Y * M[1][2] + P.Z * M[2][2] + P.W * M[3][2];
    Result.W = P.X * M[0][3] + P.Y * M[1][3] + P.Z * M[2][3] + P.W * M[3][3];
    return Result;
  }

  inline FVector4 TransformFVector(const FVector& V) const
  {
    return TransformFVector4(FVector4(V.X, V.Y, V.Z, 1.0f));
  }

  inline FVector4 TransformNormal(const FVector& V) const
  {
    return TransformFVector4(FVector4(V.X, V.Y, V.Z, 0.0f));
  }

  inline float Determinant() const
  {
    return	M[0][0] * (
      M[1][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
      M[2][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
      M[3][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
      ) -
      M[1][0] * (
        M[0][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
        M[2][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
        M[3][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
        ) +
      M[2][0] * (
        M[0][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
        M[1][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
        M[3][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
        ) -
      M[3][0] * (
        M[0][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
        M[1][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
        M[2][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
        );
  }

  inline FMatrix Inverse() const
  {
    FMatrix Result;
    const float	Det = Determinant();

    if (Det == 0.0f)
      return FMatrix::Identity;

    const float	RDet = 1.0f / Det;

    Result.M[0][0] = RDet * (
      M[1][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
      M[2][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
      M[3][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
      );
    Result.M[0][1] = -RDet * (
      M[0][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
      M[2][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
      M[3][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
      );
    Result.M[0][2] = RDet * (
      M[0][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
      M[1][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
      M[3][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
      );
    Result.M[0][3] = -RDet * (
      M[0][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
      M[1][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
      M[2][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
      );

    Result.M[1][0] = -RDet * (
      M[1][0] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
      M[2][0] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
      M[3][0] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
      );
    Result.M[1][1] = RDet * (
      M[0][0] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
      M[2][0] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
      M[3][0] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
      );
    Result.M[1][2] = -RDet * (
      M[0][0] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
      M[1][0] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
      M[3][0] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
      );
    Result.M[1][3] = RDet * (
      M[0][0] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
      M[1][0] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
      M[2][0] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
      );

    Result.M[2][0] = RDet * (
      M[1][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
      M[2][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) +
      M[3][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1])
      );
    Result.M[2][1] = -RDet * (
      M[0][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
      M[2][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
      M[3][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1])
      );
    Result.M[2][2] = RDet * (
      M[0][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) -
      M[1][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
      M[3][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
      );
    Result.M[2][3] = -RDet * (
      M[0][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1]) -
      M[1][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1]) +
      M[2][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
      );

    Result.M[3][0] = -RDet * (
      M[1][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
      M[2][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) +
      M[3][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
      );
    Result.M[3][1] = RDet * (
      M[0][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
      M[2][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
      M[3][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1])
      );
    Result.M[3][2] = -RDet * (
      M[0][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) -
      M[1][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
      M[3][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
      );
    Result.M[3][3] = RDet * (
      M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1]) -
      M[1][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1]) +
      M[2][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
      );

    return Result;
  }

  inline FMatrix operator*(float scale) const
  {
    FMatrix ResultMat;

    for (int32 X = 0; X < 4; X++)
    {
      for (int32 Y = 0; Y < 4; Y++)
      {
        ResultMat.M[X][Y] = M[X][Y] * scale;
      }
    }

    return ResultMat;
  }

  inline void operator*=(float Other)
  {
    *this = *this * Other;
  }

  void operator*=(const FMatrix& b);

  FMatrix operator*(const FMatrix& a) const;

  FVector Rotate(FVector& v);

  struct FRotator Rotator() const;

  FVector GetOrigin() const;

  void ScaleTranslation(const FVector& scale3D);

  float M[4][4];
};

struct FTranslationMatrix : FMatrix {
  FTranslationMatrix(const FVector& d) :
    FMatrix(
      FPlane(1.0f, 0.0f, 0.0f, 0.0f),
      FPlane(0.0f, 1.0f, 0.0f, 0.0f),
      FPlane(0.0f, 0.0f, 1.0f, 0.0f),
      FPlane(d.X, d.Y, d.Z, 1.0f))
  {}
};

struct FRotator;
struct FRotationTranslationMatrix : FMatrix {
  FRotationTranslationMatrix(const FRotator& Rot, const FVector& Origin);
};

struct FRotationMatrix : FRotationTranslationMatrix {
  FRotationMatrix(const FRotator& Rot)
    : FRotationTranslationMatrix(Rot, FVector())
  {}
};

struct FQuat {
  static const FQuat Identity;

  float X = 0;
  float Y = 0;
  float Z = 0;
  float W = 0;

  FQuat() = default;

  FQuat(float x, float y, float z, float w)
    : X(x)
    , Y(y)
    , Z(z)
    , W(w)
  {}

  FQuat(const FMatrix& matrix);

  float& operator[](int32 idx)
  {
    switch (idx)
    {
    case 0:
      return X;
    case 1:
      return Y;
    case 2:
      return Z;
    }
    return W;
  }

  const float& operator[](int32 idx) const
  {
    switch (idx)
    {
    case 0:
      return X;
    case 1:
      return Y;
    case 2:
      return Z;
    }
    return W;
  }

  FQuat Inverse() const;

  FVector Euler() const;

  FVector FbxEuler() const;

  FRotator Rotator() const;

  void Normalize(float tolerance = 1.e-8)
  {
    const float ss = X * X + Y * Y + Z * Z + W * W;
    if (ss > tolerance)
    {
      const float s = 1. / sqrt(ss);
      X *= s;
      Y *= s;
      Z *= s;
      W *= s;
    }
    else
    {
      *this = FQuat::Identity;
    }
  }

  friend FStream& operator<<(FStream& s, FQuat& f);
};

struct FQuatRotationTranslationMatrix : public FMatrix {
  FQuatRotationTranslationMatrix(const FQuat& q, const FVector& origin);
};

struct FRotator
{
  int32 Pitch = 0; // Looking up and down (0=Straight Ahead, +Up, -Down).
  int32 Yaw = 0;   // Rotating around (running in circles), 0=East, +North, -South.
  int32 Roll = 0;  // Rotation about axis of screen, 0=Straight, +Clockwise, -CCW.

  FRotator() = default;

  FRotator(int32 pitch, int32 yaw, int32 roll)
    : Pitch(pitch)
    , Yaw(yaw)
    , Roll(roll)
  {}

  FRotator(const FRotator& r) = default;

  FRotator operator+(const FRotator& r) const
  {
    return FRotator(Pitch + r.Pitch, Yaw + r.Yaw, Roll + r.Roll);
  }

  static int32 NormalizeAxis(int32 a)
  {
    a &= 0xFFFF;
    if (a > 32767)
    {
      a -= 0x10000;
    }
    return a;
  }

  inline bool IsZero() const
  {
    return !Pitch && !Yaw && !Roll;
  }

  static FRotator MakeFromEuler(const FVector& Euler)
  {
    return FRotator((int32)truncf(Euler.Y * (32768.f / 180.f)), (int32)truncf(Euler.Z * (32768.f / 180.f)), (int32)truncf(Euler.X * (32768.f / 180.f)));
  }

  FRotator Normalized() const;
  FRotator Denormalized() const;
  FVector Euler() const;
  FQuat Quaternion() const;
  FRotator GetInverse() const;

  friend FStream& operator<<(FStream& s, FRotator& r);
};

struct FPackedNormal
{
  union
  {
    struct
    {
      uint8	X, Y, Z, W;
    };
    uint32 Packed = 0;
  } Vector;

  FPackedNormal() = default;
  FPackedNormal(const FVector& vec)
  {
    *this = vec;
  }

  operator FVector() const;

  void operator=(const FVector& vec);
  void operator=(const FVector4& vec);

  friend FStream& operator<<(FStream& s, FPackedNormal& n);
};

struct FWordBulkData : public FUntypedBulkData
{
  int32 GetElementSize() const override
  {
    return sizeof(uint16);
  }

  void SerializeElement(FStream& s, void* data, int32 elementIndex) override;
};

struct FIntBulkData : public FUntypedBulkData
{
  int32 GetElementSize() const override
  {
    return sizeof(uint32);
  }

  void SerializeElement(FStream& s, void* data, int32 elementIndex) override;
};

class FFloat32
{
public:
  union
  {
    struct
    {
      uint32 Mantissa : 23;
      uint32 Exponent : 8;
      uint32 Sign : 1;
    } Components;
    float	FloatValue = 0;
  };

  FFloat32(float v = 0.0f)
    : FloatValue(v)
  {}
};


class FFloat16
{
public:
  union
  {
    struct
    {
      uint16 Mantissa : 10;
      uint16 Exponent : 5;
      uint16 Sign : 1;
    } Components;
    uint16 Packed = 0;
  };

  FFloat16()
    : Packed(0)
  {}

  FFloat16(const FFloat16& v)
  {
    Packed = v.Packed;
  }

  FFloat16(float v)
  {
    Set(v);
  }

  operator float() const
  {
    return GetFloat();
  }

  FFloat16& operator=(float v)
  {
    Set(v);
    return *this;
  }

  FFloat16& operator=(const FFloat16& v)
  {
    Packed = v.Packed;
    return *this;
  }

  void Set(float v32)
  {
    FFloat32 ffv(v32);

    Components.Sign = ffv.Components.Sign;

    if (ffv.Components.Exponent <= 112)
    {
      Components.Exponent = 0;
      Components.Mantissa = 0;
    }
    else if (ffv.Components.Exponent >= 143)
    {
      Components.Exponent = 30;
      Components.Mantissa = 1023;
    }
    else
    {
      Components.Exponent = int32(ffv.Components.Exponent) - 127 + 15;
      Components.Mantissa = uint16(ffv.Components.Mantissa >> 13);
    }
  }

  float GetFloat() const
  {
    FFloat32	result;
    result.Components.Sign = Components.Sign;
    if (Components.Exponent == 0)
    {
      result.Components.Exponent = 0;
      result.Components.Mantissa = 0;
    }
    else if (Components.Exponent == 31)
    {
      result.Components.Exponent = 142;
      result.Components.Mantissa = 8380416;
    }
    else
    {
      result.Components.Exponent = int32(Components.Exponent) - 15 + 127;
      result.Components.Mantissa = uint32(Components.Mantissa) << 13;
    }
    return result.FloatValue;
  }

  friend FStream& operator<<(FStream& s, FFloat16& v);
};

class FFloatInfo {
public:
  enum { MantissaBits = 23 };
  enum { ExponentBits = 8 };
  enum { SignShift = 31 };
  enum { ExponentBias = 127 };

  enum { MantissaMask = 0x007fffff };
  enum { ExponentMask = 0x7f800000 };
  enum { SignMask = 0x80000000 };

  typedef float FloatType;
  typedef uint32 PackedType;

  static PackedType ToPackedType(FloatType Value)
  {
    return *(PackedType*)&Value;
  }

  static FloatType ToFloatType(PackedType Value)
  {
    return *(FloatType*)&Value;
  }
};

template<uint32 TExpCount, uint32 TMantCount, bool TRound, typename FloatInfo = FFloatInfo>
class TPackedFloat
{
public:
  enum { NumOutputsBits = TExpCount + TMantCount + 1 };

  enum { MantissaShift = FloatInfo::MantissaBits - TMantCount };
  enum { ExponentBias = (1 << (TExpCount - 1)) - 1 };
  enum { SignShift = TExpCount + TMantCount };

  enum { MantissaMask = (1 << TMantCount) - 1 };
  enum { ExponentMask = ((1 << TExpCount) - 1) << TMantCount };
  enum { SignMask = 1 << SignShift };

  enum { MinExponent = -ExponentBias - 1 };
  enum { MaxExponent = ExponentBias };

  typedef typename FloatInfo::PackedType  PackedType;
  typedef typename FloatInfo::FloatType  FloatType;

  PackedType Encode(FloatType v) const
  {
    if (v == (FloatType)0.0)
    {
      return (PackedType)0;
    }

    const PackedType packed = FloatInfo::ToPackedType(v);
    PackedType  mantissa = packed & FloatInfo::MantissaMask;
    int32 exponent = (packed & FloatInfo::ExponentMask) >> FloatInfo::MantissaBits;
    const PackedType sign = packed >> FloatInfo::SignShift;

    exponent -= FloatInfo::ExponentBias;

    if (TRound)
    {
      mantissa += (1 << (MantissaShift - 1));
      if (mantissa & (1 << FloatInfo::MantissaBits))
      {
        mantissa = 0;
        ++exponent;
      }
    }

    mantissa >>= MantissaShift;

    if (exponent < MinExponent)
    {
      return (PackedType)0;
    }
    if (exponent > MaxExponent)
    {
      exponent = MaxExponent;
    }

    exponent -= MinExponent;
    return (sign << SignShift) | (exponent << TMantCount) | (mantissa);
  }

  FloatType Decode(PackedType v) const
  {
    if (v == (PackedType)0)
    {
      return (FloatType)0.0;
    }

    PackedType mantissa = v & MantissaMask;
    int32 exponent = (v & ExponentMask) >> TMantCount;
    const PackedType sign = v >> SignShift;
    exponent += MinExponent;
    exponent += FloatInfo::ExponentBias;
    mantissa <<= MantissaShift;
    return FloatInfo::ToFloatType((sign << FloatInfo::SignShift) | (exponent << FloatInfo::MantissaBits) | (mantissa));
  }
};

struct FVector2DHalf {
  FFloat16 X = 0;
  FFloat16 Y = 0;

  FVector2DHalf() = default;

  FVector2DHalf(const FFloat16& x, const FFloat16& y)
    : X(x), Y(y)
  {}
  
  FVector2DHalf(float x, float y)
    : X(x), Y(y)
  {}
  
  FVector2DHalf(const FVector2D& v)
    : X(v.X), Y(v.Y)
  {}

  FVector2DHalf& operator=(const FVector2D& v)
  {
    X = FFloat16(v.X);
    Y = FFloat16(v.Y);
    return *this;
  }

  operator FVector2D() const
  {
    return FVector2D(X, Y);
  }

  friend FStream& operator<<(FStream& s, FVector2DHalf& v);
};

struct FPackedPosition
{
  union
  {
    struct
    {
      int32 X : 11;
      int32 Y : 11;
      int32 Z : 10;
    } Vector;
    uint32 Packed = 0;
  };

  operator FVector() const
  {
    return FVector(Vector.X / 1023.f, Vector.Y / 1023.f, Vector.Z / 511.f);
  }

  FPackedPosition& operator=(FVector v)
  {
    Set(v);
    return *this;
  }

  void Set(const FVector& inVector);

  friend FStream& operator<<(FStream& s, FPackedPosition& p);
};

class FMultiSizeIndexContainer {
public:
  ~FMultiSizeIndexContainer()
  {
    free(IndexBuffer);
  }

  FMultiSizeIndexContainer() = default;

  FMultiSizeIndexContainer(const FMultiSizeIndexContainer& a)
  {
    ElementCount = a.ElementCount;
    ElementSize = a.ElementSize;
    BulkElementSize = a.BulkElementSize;
    if (ElementCount && ElementSize)
    {
      AllocateBuffer(ElementCount, ElementSize);
      std::memcpy(IndexBuffer, a.IndexBuffer, ElementCount * ElementSize);
    }
  }

  FMultiSizeIndexContainer& operator=(const FMultiSizeIndexContainer& a)
  {
    ElementCount = a.ElementCount;
    ElementSize = a.ElementSize;
    BulkElementSize = a.BulkElementSize;
    if (ElementCount && ElementSize)
    {
      AllocateBuffer(ElementCount, ElementSize);
      std::memcpy(IndexBuffer, a.IndexBuffer, ElementCount * ElementSize);
    }
    return *this;
  }

  friend FStream& operator<<(FStream& s, FMultiSizeIndexContainer& c);

  uint8 GetElementSize() const
  {
    return ElementSize;
  }

  void SetElementSize(size_t size)
  {
    if (size == ElementSize)
    {
      return;
    }
    DBreakIf(size > sizeof(int32) || !size);
    uint8 oldSize = ElementSize;
    ElementSize = (uint8)size;
    BulkElementSize = ElementSize;
    void* oldBuffer = IndexBuffer;
    IndexBuffer = nullptr;
    if (ElementCount)
    {
      AllocateBuffer(ElementCount, ElementSize);
    }
    if (oldBuffer)
    {
      if (oldSize == sizeof(uint16))
      {
        for (uint32 idx = 0; idx < ElementCount; ++idx)
        {
          ((uint32*)IndexBuffer)[idx] = (uint32)((uint16*)oldBuffer)[idx];
        }
      }
      else
      {
        uint32 tmp = 0;
        for (uint32 idx = 0; idx < ElementCount; ++idx)
        {
          tmp = ((uint32*)oldBuffer)[idx];
          DBreakIf(tmp > 0xFFFF);
          ((uint16*)IndexBuffer)[idx] = (uint16)tmp;
        }
      }
      free(oldBuffer);
    }
  }

  uint32 GetElementCount() const
  {
    return ElementCount;
  }

  void SetIndex(uint32 elementIndex, uint32 value)
  {
    if (ElementSize == sizeof(uint16))
    {
      Get16BitBuffer()[elementIndex] = (uint16)value;
    }
    else
    {
      Get32BitBuffer()[elementIndex] = value;
    }
  }

  void AllocateBuffer(uint32 elementCount, uint8 elementSize)
  {
    free(IndexBuffer);
    IndexBuffer = nullptr;
    ElementSize = elementSize;
    ElementCount = elementCount;
    if (elementCount * elementSize)
    {
      IndexBuffer = malloc(elementCount * elementSize);
    }
  }

  uint16* Get16BitBuffer()
  {
    return (uint16*)IndexBuffer;
  }

  uint32* Get32BitBuffer()
  {
    return (uint32*)IndexBuffer;
  }

  uint32 GetIndex(int32 elementIndex) const
  {
    if (ElementSize == sizeof(uint16))
    {
      uint16* tmp = (uint16*)IndexBuffer;
      return (uint32) * (tmp + elementIndex);
    }
    uint32* tmp = (uint32*)IndexBuffer;
    return *(tmp + elementIndex);
  }

  void AddIndex(int32 elementIndex)
  {
    ElementCount++;
    if (!IndexBuffer)
    {
      IndexBuffer = malloc(ElementCount * ElementSize);
    }
    else
    {
      if (void* tmp = realloc(IndexBuffer, ElementCount * ElementSize))
      {
        IndexBuffer = tmp;
      }
      else
      {
        DBreak();
        ElementCount--;
        return;
      }
    }
    if (ElementSize == sizeof(uint16))
    {
      Get16BitBuffer()[ElementCount - 1] = elementIndex;
    }
    else
    {
      Get32BitBuffer()[ElementCount - 1] = elementIndex;
    }
  }

  friend FStream& operator<<(FStream& s, FMultiSizeIndexContainer& c);

private:
  uint8 ElementSize = 2;
  uint32 BulkElementSize = 2; // Not an actual field of the MultiSizeContainer. Belongs to a bulk array serialization. TODO: create a template for bulk array serialization
  uint32 ElementCount = 0;
  bool NeedsCPUAccess = true;
  void* IndexBuffer = nullptr;
};

class FRawIndexBuffer {
public:
  FRawIndexBuffer() = default;

  FRawIndexBuffer(const FRawIndexBuffer& a)
  {
    ElementCount = a.ElementCount;
    ElementSize = a.ElementSize;
    if (ElementSize && ElementCount)
    {
      AllocateBuffer(ElementCount, ElementSize);
      std::memcpy(Data, a.Data, ElementCount * ElementSize);
    }
  }

  ~FRawIndexBuffer()
  {
    free(Data);
  }

  friend FStream& operator<<(FStream& s, FRawIndexBuffer& b);

  void SortIndices();

  uint32 GetElementSize() const
  {
    return ElementSize;
  }

  uint32 GetElementCount() const
  {
    return ElementCount;
  }

  void AllocateBuffer(uint32 elementCount, uint8 elementSize)
  {
    free(Data);
    ElementSize = elementSize;
    ElementCount = elementCount;
    if (elementCount * elementSize)
    {
      Data = malloc(elementCount * elementSize);
    }
  }

  uint16* Get16BitBuffer()
  {
    return (uint16*)Data;
  }

  uint32* Get32BitBuffer()
  {
    return (uint32*)Data;
  }

  uint32 GetIndex(int32 elementIndex) const
  {
    if (ElementSize == sizeof(uint16))
    {
      uint16* tmp = (uint16*)Data;
      return (uint32) * (tmp + elementIndex);
    }
    uint32* tmp = (uint32*)Data;
    return *(tmp + elementIndex);
  }

  void SetIndex(uint32 elementIndex, uint32 value)
  {
    if (ElementSize == sizeof(uint16))
    {
      Get16BitBuffer()[elementIndex] = (uint16)value;
    }
    else
    {
      Get32BitBuffer()[elementIndex] = value;
    }
  }

  void AddIndex(int32 elementIndex)
  {
    ElementCount++;
    if (!Data)
    {
      Data = malloc(ElementCount * ElementSize);
    }
    else
    {
      if (void* tmp = realloc(Data, ElementCount * ElementSize))
      {
        Data = tmp;
      }
      else
      {
        DBreak();
        ElementCount--;
        return;
      }
    }
    if (ElementSize == sizeof(uint16))
    {
      Get16BitBuffer()[ElementCount - 1] = elementIndex;
    }
    else
    {
      Get32BitBuffer()[ElementCount - 1] = elementIndex;
    }
  }

protected:
  uint32 ElementSize = 2;
  uint32 ElementCount = 0;
  void* Data = nullptr;
};

struct FSphere {
  FVector Center;
  float Radius = 1.;

  friend FStream& operator<<(FStream& s, FSphere& sp);
};

struct FStreamableTextureInstance {
  FSphere BoundingSphere;
  float TexelFactor = 1.;

  friend FStream& operator<<(FStream& s, FStreamableTextureInstance& i);
};

struct FDynamicTextureInstance : public FStreamableTextureInstance
{
  UObject* Texture = nullptr;
  bool bAttached = false;
  float OriginalRadius = 1.;

  friend FStream& operator<<(FStream& s, FDynamicTextureInstance& i);
};

struct FCachedPhysSMData {
  FVector Scale3D;
  int32 CachedDataIndex = 0;

  friend FStream& operator<<(FStream& s, FCachedPhysSMData& d);
};

struct FCachedPerTriPhysSMData {
  FVector Scale3D;
  int32 CachedDataIndex = 0;

  friend FStream& operator<<(FStream& s, FCachedPerTriPhysSMData& d);
};

struct FKCachedPerTriData {
  std::vector<uint8>	CachedPerTriData;
  FILE_OFFSET CachedPerTriDataElementSize = 1;

  friend FStream& operator<<(FStream& s, FKCachedPerTriData& d);
};

struct FVolumeLightingSampleInfo {
  FBox Bounds;
  float SampleSpacing = 0.;

};

struct FVolumeLightingSample {
  FVector Position;
  float Radius = 0;

  uint8 IndirectDirectionTheta = 0;
  uint8 IndirectDirectionPhi = 0;

  uint8 EnvironmentDirectionTheta = 0;
  uint8 EnvironmentDirectionPhi = 0;

  FColor IndirectRadiance;
  FColor EnvironmentRadiance;
  FColor AmbientRadiance;

  uint8 bShadowedFromDominantLights = 0;

  friend FStream& operator<<(FStream& s, FVolumeLightingSample& ls);
};

struct FPrecomputedLightVolume {
  bool bInitialized = false;
  FBox Bounds;
  float SampleSpacing = 0.;
  std::vector<FVolumeLightingSample> Samples;

  friend FStream& operator<<(FStream& s, FPrecomputedLightVolume& v);
};

struct FPrecomputedVisibilityCell {
  FVector Min;
  uint16 ChunkIndex = 0;
  uint16 DataOffset = 0;

  friend FStream& operator<<(FStream& s, FPrecomputedVisibilityCell& c);
};

struct FCompressedVisibilityChunk {
  bool bCompressed = false;
  int32 UncompressedSize = 0;
  std::vector<uint8> Data;

  friend FStream& operator<<(FStream& s, FCompressedVisibilityChunk& c);
};

struct FPrecomputedVisibilityBucket {
  int32 CellDataSize = 0;
  std::vector<FPrecomputedVisibilityCell> Cells;
  std::vector<FCompressedVisibilityChunk> CellDataChunks;

  friend FStream& operator<<(FStream& s, FPrecomputedVisibilityBucket& b);
};

struct FPrecomputedVisibilityHandler {
  FVector2D BucketOriginXY;
  float SizeXY = 0.;
  float SizeZ = 0.;
  int32	BucketSizeXY = 0;
  int32	NumBuckets = 0;
  std::vector<FPrecomputedVisibilityBucket> Buckets;

  friend FStream& operator<<(FStream& s, FPrecomputedVisibilityHandler& h);
};

struct FPrecomputedVolumeDistanceField {
  float VolumeMaxDistance = 0;
  FBox VolumeBox;
  int32 VolumeSizeX = 0;
  int32 VolumeSizeY = 0;
  int32 VolumeSizeZ = 0;
  std::vector<FColor> Data;

  friend FStream& operator<<(FStream& s, FPrecomputedVolumeDistanceField& f);
};

struct FEdge {
  int32 Vertex[2] = { 0, 0 };
  int32 Face[2] = { 0, 0 };

  friend FStream& operator<<(FStream& s, FEdge& f);
};

struct FQuatFixed48NoW {
  uint16 X = 0;
  uint16 Y = 0;
  uint16 Z = 0;

  FQuatFixed48NoW() = default;

  FQuatFixed48NoW(const FQuat& q)
  {
    FromQuat(q);
  }

  void FromQuat(const FQuat& q)
  {
    FQuat t(q);
    if (t.W < 0.f)
    {
      t.X = -t.X;
      t.Y = -t.Y;
      t.Z = -t.Z;
      t.W = -t.W;
    }
    t.Normalize();

    X = (int32)(t.X * Quant16BitFactor) + Quant16BitOffs;
    Y = (int32)(t.Y * Quant16BitFactor) + Quant16BitOffs;
    Z = (int32)(t.Z * Quant16BitFactor) + Quant16BitOffs;
  }

  void ToQuat(FQuat& q) const
  {
    const float fX = ((int32)X - (int32)Quant16BitOffs) / Quant16BitDiv;
    const float fY = ((int32)Y - (int32)Quant16BitOffs) / Quant16BitDiv;
    const float fZ = ((int32)Z - (int32)Quant16BitOffs) / Quant16BitDiv;
    const float w = 1.f - fX * fX - fY * fY - fZ * fZ;

    q.X = fX;
    q.Y = fY;
    q.Z = fZ;
    q.W = w > 0.f ? sqrt(w) : 0.f;
  }

  friend FStream& operator<<(FStream& s, FQuatFixed48NoW& q);
};

struct FQuatFixed32NoW {
  uint32 Packed = 0;

  FQuatFixed32NoW() = default;

  FQuatFixed32NoW(const FQuat& q)
  {
    FromQuat(q);
  }

  void FromQuat(const FQuat& q)
  {
    FQuat t(q);
    if (t.W < 0.f)
    {
      t.X = -t.X;
      t.Y = -t.Y;
      t.Z = -t.Z;
      t.W = -t.W;
    }
    t.Normalize();

    const uint32 packedX = (int32)(t.X * Quant11BitFactor) + Quant11BitOffs;
    const uint32 packedY = (int32)(t.Y * Quant11BitFactor) + Quant11BitOffs;
    const uint32 packedZ = (int32)(t.Z * Quant10BitFactor) + Quant10BitOffs;

    const uint32 xShift = 21;
    const uint32 yShift = 10;
    Packed = (packedX << xShift) | (packedY << yShift) | (packedZ);
  }

  void ToQuat(FQuat& result) const
  {
    const uint32 xShift = 21;
    const uint32 yShift = 10;
    const uint32 zMask = 0x000003ff;
    const uint32 yMask = 0x001ffc00;
    const uint32 xMask = 0xffe00000;

    const uint32 unpackedX = Packed >> xShift;
    const uint32 unpackedY = (Packed & yMask) >> yShift;
    const uint32 unpackedZ = (Packed & zMask);

    const float X = ((int32)unpackedX - (int32)Quant11BitOffs) / Quant11BitDiv;
    const float Y = ((int32)unpackedY - (int32)Quant11BitOffs) / Quant11BitDiv;
    const float Z = ((int32)unpackedZ - (int32)Quant10BitOffs) / Quant10BitDiv;
    const float wSquared = 1.f - X * X - Y * Y - Z * Z;

    result.X = X;
    result.Y = Y;
    result.Z = Z;
    result.W = wSquared > 0.f ? sqrt(wSquared) : 0.f;
  }

  friend FStream& operator<<(FStream& s, FQuatFixed32NoW& q);
};

struct FQuatFloat96NoW {
  float X = 0;
  float Y = 0;
  float Z = 0;

  FQuatFloat96NoW() = default;

  FQuatFloat96NoW(const FQuat& q)
  {
    FromQuat(q);
  }

  FQuatFloat96NoW(float x, float y, float z)
    : X(x)
    , Y(y)
    , Z(z)
  {}

  void FromQuat(const FQuat& q)
  {
    FQuat temp(q);
    if (temp.W < 0.f)
    {
      temp.X = -temp.X;
      temp.Y = -temp.Y;
      temp.Z = -temp.Z;
      temp.W = -temp.W;
    }
    temp.Normalize();
    X = temp.X;
    Y = temp.Y;
    Z = temp.Z;
  }

  void ToQuat(FQuat& q) const
  {
    const float w = 1.f - X * X - Y * Y - Z * Z;

    q.X = X;
    q.Y = Y;
    q.Z = Z;
    q.W = w > 0.f ? sqrt(w) : 0.f;
  }

  friend FStream& operator<<(FStream& s, FQuatFloat96NoW& q);
};



struct FVectorFixed48 {
  uint16 X = 0;
  uint16 Y = 0;
  uint16 Z = 0;

  FVectorFixed48() = default;

  FVectorFixed48(const FVector& v)
  {
    FromVector(v);
  }

  void FromVector(const FVector& v)
  {
    FVector temp(v / 128.0f);

    X = (int32)(temp.X * Quant16BitFactor) + Quant16BitOffs;
    Y = (int32)(temp.Y * Quant16BitFactor) + Quant16BitOffs;
    Z = (int32)(temp.Z * Quant16BitFactor) + Quant16BitOffs;
  }

  void ToVector(FVector& result) const
  {
    const float fX = ((int32)X - (int32)Quant16BitOffs) / Quant16BitDiv;
    const float fY = ((int32)Y - (int32)Quant16BitOffs) / Quant16BitDiv;
    const float fZ = ((int32)Z - (int32)Quant16BitOffs) / Quant16BitDiv;

    result.X = fX * 128.0f;
    result.Y = fY * 128.0f;
    result.Z = fZ * 128.0f;
  }

  friend FStream& operator<<(FStream& s, FVectorFixed48& v);
};

struct FVectorIntervalFixed32NoW {
  uint32 Packed = 0;

  FVectorIntervalFixed32NoW() = default;

  FVectorIntervalFixed32NoW(const FVector& v, const float* mins, const float* ranges)
  {
    FromVector(v, mins, ranges);
  }

  void FromVector(const FVector& v, const float* mins, const float* ranges)
  {
    FVector tmp(v);

    tmp.X -= mins[0];
    tmp.Y -= mins[1];
    tmp.Z -= mins[2];

    const uint32 pX = (int32)((tmp.X / ranges[0]) * Quant10BitFactor) + Quant10BitOffs;
    const uint32 pY = (int32)((tmp.Y / ranges[1]) * Quant11BitFactor) + Quant11BitOffs;
    const uint32 pZ = (int32)((tmp.Z / ranges[2]) * Quant11BitFactor) + Quant11BitOffs;

    const uint32 ZShift = 21;
    const uint32 YShift = 10;
    Packed = (pZ << ZShift) | (pY << YShift) | (pX);
  }

  void ToVector(FVector& result, const float* mins, const float* ranges) const
  {
    const uint32 ZShift = 21;
    const uint32 YShift = 10;
    const uint32 XMask = 0x000003ff;
    const uint32 YMask = 0x001ffc00;
    const uint32 ZMask = 0xffe00000;

    const uint32 uZ = Packed >> ZShift;
    const uint32 uY = (Packed & YMask) >> YShift;
    const uint32 uX = (Packed & XMask);

    const float X = ((((int32)uX - (int32)Quant10BitOffs) / Quant10BitDiv) * ranges[0] + mins[0]);
    const float Y = ((((int32)uY - (int32)Quant11BitOffs) / Quant11BitDiv) * ranges[1] + mins[1]);
    const float Z = ((((int32)uZ - (int32)Quant11BitOffs) / Quant11BitDiv) * ranges[2] + mins[2]);

    result.X = X;
    result.Y = Y;
    result.Z = Z;
  }

  friend FStream& operator<<(FStream& s, FVectorIntervalFixed32NoW& v);
};


struct FQuatIntervalFixed32NoW {
  uint32 Packed = 0;

  FQuatIntervalFixed32NoW() = default;

  FQuatIntervalFixed32NoW(const FQuat& q, const float* mins, const float* ranges)
  {
    FromQuat(q, mins, ranges);
  }

  void FromQuat(const FQuat& q, const float* mins, const float* ranges)
  {
    FQuat t(q);
    if (t.W < 0.f)
    {
      t.X = -t.X;
      t.Y = -t.Y;
      t.Z = -t.Z;
      t.W = -t.W;
    }
    t.Normalize();

    t.X -= mins[0];
    t.Y -= mins[1];
    t.Z -= mins[2];

    const uint32 PackedX = (int32)((t.X / ranges[0]) * Quant11BitFactor) + Quant11BitOffs;
    const uint32 PackedY = (int32)((t.Y / ranges[1]) * Quant11BitFactor) + Quant11BitOffs;
    const uint32 PackedZ = (int32)((t.Z / ranges[2]) * Quant10BitFactor) + Quant10BitOffs;

    const uint32 XShift = 21;
    const uint32 YShift = 10;
    Packed = (PackedX << XShift) | (PackedY << YShift) | (PackedZ);
  }

  void ToQuat(FQuat& result, const float* mins, const float* ranges) const
  {
    const uint32 XShift = 21;
    const uint32 YShift = 10;
    const uint32 ZMask = 0x000003ff;
    const uint32 YMask = 0x001ffc00;
    const uint32 XMask = 0xffe00000;

    const uint32 UnpackedX = Packed >> XShift;
    const uint32 UnpackedY = (Packed & YMask) >> YShift;
    const uint32 UnpackedZ = (Packed & ZMask);

    const float X = ((((int32)UnpackedX - (int32)Quant11BitOffs) / Quant11BitDiv) * ranges[0] + mins[0]);
    const float Y = ((((int32)UnpackedY - (int32)Quant11BitOffs) / Quant11BitDiv) * ranges[1] + mins[1]);
    const float Z = ((((int32)UnpackedZ - (int32)Quant10BitOffs) / Quant10BitDiv) * ranges[2] + mins[2]);
    const float WSquared = 1.f - X * X - Y * Y - Z * Z;

    result.X = X;
    result.Y = Y;
    result.Z = Z;
    result.W = WSquared > 0.f ? sqrt(WSquared) : 0.f;
  }

  friend FStream& operator<<(FStream& s, FQuatIntervalFixed32NoW& q);
};

struct FQuatFloat32NoW {
  uint32 Packed = 0;

  FQuatFloat32NoW() = default;

  FQuatFloat32NoW(const FQuat& q)
  {
    FromQuat(q);
  }

  void FromQuat(const FQuat& q)
  {
    FQuat tmp(q);
    if (tmp.W < 0.f)
    {
      tmp.X = -tmp.X;
      tmp.Y = -tmp.Y;
      tmp.Z = -tmp.Z;
      tmp.W = -tmp.W;
    }
    tmp.Normalize();

    TPackedFloat<3, 7, true> packed37;
    TPackedFloat<3, 6, true> packed36;

    const uint32 px = packed37.Encode(tmp.X);
    const uint32 py = packed37.Encode(tmp.Y);
    const uint32 pz = packed36.Encode(tmp.Z);

    const uint32 XShift = 21;
    const uint32 YShift = 10;
    Packed = (px << XShift) | (py << YShift) | (pz);
  }

  void ToQuat(FQuat& result) const
  {
    const uint32 XShift = 21;
    const uint32 YShift = 10;
    const uint32 ZMask = 0x000003ff;
    const uint32 YMask = 0x001ffc00;
    const uint32 XMask = 0xffe00000;

    const uint32 ux = Packed >> XShift;
    const uint32 uy = (Packed & YMask) >> YShift;
    const uint32 uz = (Packed & ZMask);

    TPackedFloat<3, 7, true> packed37;
    TPackedFloat<3, 6, true> packed36;

    const float X = packed37.Decode(ux);
    const float Y = packed37.Decode(uy);
    const float Z = packed36.Decode(uz);
    const float WSquared = 1.f - X * X - Y * Y - Z * Z;

    result.X = X;
    result.Y = Y;
    result.Z = Z;
    result.W = WSquared > 0.f ? sqrt(WSquared) : 0.f;
  }

  friend FStream& operator<<(FStream& s, FQuatFloat32NoW& q);
};