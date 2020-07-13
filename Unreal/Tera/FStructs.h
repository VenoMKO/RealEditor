#pragma once
#include "Core.h"

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

	friend FStream& operator<<(FStream& s, FGuid& g);

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

struct FPackageSummary {
	uint32 Magic = PACKAGE_MAGIC;
	uint16 FileVersion = 610;
	uint16 LicenseeVersion = 14;
	int32 HeaderSize = 0;
	std::string FolderName = "None";
	uint32 PackageFlags = 0;
	uint32 NamesCount = 0;
	FILE_OFFSET NamesOffset = 0;
	uint32 ExportsCount = 0;
	FILE_OFFSET ExportsOffset = 0;
	uint32 ImportsCount = 0;
	FILE_OFFSET ImportsOffset = 0;
	FILE_OFFSET DependsOffset = 0;
	FGuid Guid;
	std::vector<FGeneration> Generations;
	int32 EngineVersion = 4206;
	int32 ContentVersion = 76;
	uint32 CompressionFlags = 0;
	uint32 PackageSource = 0;
	std::vector<FCompressedChunk> CompressedChunks;
	std::vector<std::string> AdditionalPackagesToCook;

	// Path to the package
	std::string SourcePath;
	// Path to the decompressed package. Equals to the SourcePath if a package is not compressed
	std::string DataPath;

	std::string PackageName = "Untitled.gpk";

	friend FStream& operator<<(FStream& s, FPackageSummary& sum);
};