#pragma once
#include "Core.h"
#include "FString.h"
#include "FName.h"

struct FGuid
{
public:
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
	FPackageSummary()
	{}

	uint32 Magic = PACKAGE_MAGIC;
	uint32 FileVersion = 0;
	int32 HeaderSize = 0;
	FString FolderName = "None";
	uint32 PackageFlags = 0;
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
	uint32 PackageSource = 0;
	std::vector<FCompressedChunk> CompressedChunks;
	std::vector<FString> AdditionalPackagesToCook;
	FTextureAllocations TextureAllocations;

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

struct FVector {
	FVector()
	{}

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

	bool operator==(const FVector& v) const
	{
		return X == v.X && Y == v.Y && Z == v.Z;
	}

	float X = 0;
	float Y = 0;
	float Z = 0;
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

protected:
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

	bool operator==(const FColor& v) const
	{
		return R == v.R && G == v.G && B == v.B && A == v.A;
	}

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

	bool operator==(const FLinearColor& v) const
	{
		return R == v.R && G == v.G && B == v.B && A == v.A;
	}

	bool operator!=(const FLinearColor& v) const
	{
		return R != v.R || G != v.G || B != v.B || A != v.A;
	}

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