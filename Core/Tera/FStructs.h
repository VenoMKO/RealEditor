#pragma once
#include "Core.h"
#include "FString.h"
#include "FName.h"

struct FGuid
{
public:

  static FGuid Generate();

  FGuid()
  {}

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

  FVector operator^(const FVector& V) const
  {
    return FVector
    (
      Y * V.Z - Z * V.Y,
      Z * V.X - X * V.Z,
      X * V.Y - Y * V.X
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

  FVector operator-(const FVector& v)
  {
    return FVector(X - v.X, Y - v.Y, Z - v.Z);
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

  friend FStream& operator<<(FStream& s, FVector& v);

  float X = 0;
  float Y = 0;
  float Z = 0;
};

struct FVector4 {
  FVector4()
  {}

  FVector4(float x, float y, float z, float w)
    : X(x)
    , Y(y)
    , Z(z)
    , W(w)
  {}

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
  FBulkDataInfo()
  {}

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
  FUntypedBulkData()
  {}

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

  FColor(uint8 r, uint8 g, uint8 b, uint8 a)
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

  friend FStream& operator<<(FStream& s, FBoxSphereBounds& bounds);
};

struct FPlane : FVector {
  FPlane() = default;

  FPlane(float x, float y, float z, float w)
    : FVector(x, y, z)
    , W(w)
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

  void operator*=(const FMatrix& b);

  FVector Rotate(FVector& v);

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

  FQuat Inverse() const;

  FRotator Rotator() const;

  friend FStream& operator<<(FStream& s, FQuat& f);
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

  operator FVector() const;

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

#if _DEBUG
  float Value = 0;
#endif

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

struct FVector2DHalf {
  FFloat16 X = 0;
  FFloat16 Y = 0;

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

  friend FStream& operator<<(FStream& s, FMultiSizeIndexContainer& c);

  uint8 GetElementSize() const
  {
    return ElementSize;
  }

  uint32 GetElementCount() const
  {
    return ElementCount;
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

  ~FRawIndexBuffer()
  {
    free(Data);
  }

  friend FStream& operator<<(FStream& s, FRawIndexBuffer& b);

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
    Data = nullptr;
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
