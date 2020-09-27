#pragma once
#include "UObject.h"

#include <unordered_map>

enum { MAX_INFLUENCES = 4 };

enum ETriangleSortOption : uint8
{
	TRISORT_None = 0,
	TRISORT_CenterRadialDistance,
	TRISORT_Random,
	TRISORT_Tootle,
	TRISORT_MergeContiguous,
	TRISORT_Custom,
	TRISORT_CustomLeftRight
};

struct VJointPos
{
	FQuat Orientation;
	FVector Position;

	float Length = 0;
	float XSize = 0;
	float YSize = 0;
	float ZSize = 0;

	friend FStream& operator<<(FStream& s, VJointPos& p)
	{
		return s << p.Orientation << p.Position;
	}
};

struct FMeshBone
{
	FName Name;
	uint32 Flags = 0;
	VJointPos BonePos;
	int32 ParentIndex = 0;
	int32 NumChildren = 0;
	int32 Depth = 0;

	FColor BoneColor;

	bool operator==(const FMeshBone& B) const
	{
		return(Name == B.Name);
	}

	friend FStream& operator<<(FStream& s, FMeshBone& b)
	{
		return s << b.Name << b.Flags << b.BonePos << b.NumChildren << b.ParentIndex << b.BoneColor;
	}
};

struct FBonePair
{
	FName Bones[2];

	bool operator==(const FBonePair& b) const
	{
		return Bones[0] == b.Bones[0] && Bones[1] == b.Bones[1];
	}

	bool IsMatch(const FBonePair& b) const
	{
		return	(Bones[0] == b.Bones[0] || Bones[1] == b.Bones[0]) && (Bones[0] == b.Bones[1] || Bones[1] == b.Bones[1]);
	}
};

struct FBoneIndexPair
{
	int32 BoneIdx[2];

	bool operator==(const FBoneIndexPair& b) const
	{
		return (BoneIdx[0] == b.BoneIdx[0]) && (BoneIdx[1] == b.BoneIdx[1]);
	}

	bool operator<(const FBoneIndexPair& b) const
	{
		return (BoneIdx[0] < b.BoneIdx[0]) && (BoneIdx[1] < b.BoneIdx[1]);
	}

	friend FStream& operator<<(FStream& s, FBoneIndexPair& p)
	{
		return s << p.BoneIdx[0] << p.BoneIdx[1];
	}
};

struct FSkelMeshSection
{
	uint16 MaterialIndex = 0;
	uint16 ChunkIndex = 0;
	uint32 BaseIndex = 0;
	uint32 NumTriangles = 0;
	uint8 TriangleSorting = TRISORT_None;

	friend FStream& operator<<(FStream& s, FSkelMeshSection& m)
	{
		s << m.MaterialIndex;
		s << m.ChunkIndex;
		s << m.BaseIndex;

		if (s.GetFV() == VER_TERA_CLASSIC)
		{
			uint16 ntris = (uint16)m.NumTriangles;
			s << ntris;
			m.NumTriangles = ntris;
		}
		else
		{
			s << m.NumTriangles;
		}

		if (s.GetFV() > VER_TERA_CLASSIC)
		{
			s << m.TriangleSorting;
		}
		return s;
	}
};

struct FRigidSkinVertex
{
	FVector Position;
	FPackedNormal	TangentX;	// Tangent
	FPackedNormal TangentY;	// Binormal
	FPackedNormal TangentZ;	// Normal
	FVector2D UVs[MAX_TEXCOORDS];
	FColor Color;
	uint8 Bone = 0;

	friend FStream& operator<<(FStream& s, FRigidSkinVertex& v);
};

struct FSoftSkinVertex
{
	FVector Position;
	FPackedNormal TangentX;	// Tangent
	FPackedNormal TangentY;	// Binormal
	FPackedNormal TangentZ;	// Normal
	FVector2D UVs[MAX_TEXCOORDS];
	FColor Color;
	uint8 InfluenceBones[MAX_INFLUENCES];
	uint8 InfluenceWeights[MAX_INFLUENCES];

	friend FStream& operator<<(FStream& s, FSoftSkinVertex& v);
};

struct FSkelMeshChunk {
	uint32 BaseVertexIndex = 0;
	std::vector<FRigidSkinVertex> RigidVertices;
	std::vector<FSoftSkinVertex> SoftVertices;

	std::vector<uint16> BoneMap;

	int32 NumRigidVertices = 0;
	int32 NumSoftVertices = 0;

	int32 MaxBoneInfluences = 4;

	friend FStream& operator<<(FStream& s, FSkelMeshChunk& c);
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

	friend FStream& operator<<(FStream& s, FMultiSizeIndexContainer& c);

private:
	uint8 ElementSize = 2;
	uint32 BulkElementSize = 2; // Not an actual field of the MultiSizeContainer. Belongs to a bulk array serialization. TODO: create a template for bulk array serialization
	uint32 ElementCount = 0;
	bool NeedsCPUAccess = true;
	void* IndexBuffer = nullptr;
};

struct FMeshEdge
{
	int32	Vertices[2];
	int32	Faces[2];

	friend FStream& operator<<(FStream& s, FMeshEdge& e);
};

struct FGPUSkinVertexBase {
	FPackedNormal TangentX; // Tangent
	FPackedNormal TangentZ; // Normal
	uint8 BoneIndex[MAX_INFLUENCES];
	uint8 BoneWeight[MAX_INFLUENCES];

	void Serialize(FStream& s);

	virtual ~FGPUSkinVertexBase()
	{}

	virtual FVector GetPosition() const = 0;
	virtual FVector2D GetUVs(int32 idx) const = 0;
};

template<uint32 NumTexCoords = 1>
struct FGPUSkinVertexFloatAABB : public FGPUSkinVertexBase {
	FVector Position;
	FVector2D UV[NumTexCoords];

	friend FStream& operator<<(FStream& s, FGPUSkinVertexFloatAABB& v)
	{
		v.Serialize(s);

		s << v.Position;

		for (int32 idx = 0; idx < NumTexCoords; ++idx)
		{
			s << v.UV[idx];
		}
		return s;
	}

	FVector GetPosition() const override
	{
		return Position;
	}

	FVector2D GetUVs(int32 idx) const override
	{
		return UV[idx];
	}
};

template<uint32 NumTexCoords = 1>
struct FGPUSkinVertexFloatAAB : public FGPUSkinVertexBase {
	FPackedPosition Position;
	FVector2D UV[NumTexCoords];

	friend FStream& operator<<(FStream& s, FGPUSkinVertexFloatAAB& v)
	{
		v.Serialize(s);

		s << v.Position;

		for (int32 idx = 0; idx < NumTexCoords; ++idx)
		{
			s << v.UV[idx];
		}
		return s;
	}

	FVector GetPosition() const override
	{
		return Position;
	}

	FVector2D GetUVs(int32 idx) const override
	{
		return UV[idx];
	}
};

template<uint32 NumTexCoords = 1>
struct FGPUSkinVertexFloatABB : public FGPUSkinVertexBase {
	FVector Position;
	FVector2DHalf UV[NumTexCoords];

	friend FStream& operator<<(FStream& s, FGPUSkinVertexFloatABB& v)
	{
		v.Serialize(s);

		s << v.Position;

		for (int32 idx = 0; idx < NumTexCoords; ++idx)
		{
			s << v.UV[idx];
		}
		return s;
	}

	FVector GetPosition() const override
	{
		return Position;
	}

	FVector2D GetUVs(int32 idx) const override
	{
		return UV[idx];
	}
};

template<uint32 NumTexCoords = 1>
struct FGPUSkinVertexFloatAB : public FGPUSkinVertexBase {
	FPackedPosition Position;
	FVector2DHalf UV[NumTexCoords];

	friend FStream& operator<<(FStream& s, FGPUSkinVertexFloatAB& v)
	{
		v.Serialize(s);

		s << v.Position;

		for (int32 idx = 0; idx < NumTexCoords; ++idx)
		{
			s << v.UV[idx];
		}
		return s;
	}

	FVector GetPosition() const override
	{
		return Position;
	}

	FVector2D GetUVs(int32 idx) const override
	{
		return UV[idx];
	}
};

struct  FSkeletalMeshVertexBuffer {
	bool bUseFullPrecisionUVs = true;
	bool bUsePackedPosition = false;

	uint32 NumVertices = 0;
	uint32 NumTexCoords = 1;
	FVector MeshOrigin;
	FVector MeshExtension;
	uint32 ElementSize = 0;
	uint32 ElementCount = 0;
	FGPUSkinVertexBase* Data = nullptr;

	friend FStream& operator<<(FStream& s, FSkeletalMeshVertexBuffer& b);
	
	~FSkeletalMeshVertexBuffer()
	{
		delete[] Data;
	}
};

struct  FSkeletalMeshColorBuffer {
	FColor* Data = nullptr;
	uint32 ElementSize = 0;
	uint32 ElementCount = 0;

	friend FStream& operator<<(FStream& s, FSkeletalMeshColorBuffer& b);
	
	~FSkeletalMeshColorBuffer()
	{
		delete[] Data;
	}
};

struct FInfluenceWeights
{
	union
	{
		struct
		{
			uint8 InfluenceWeights[MAX_INFLUENCES];
		};
		uint32 InfluenceWeightsUint32 = 0;
	};

	friend FStream& operator<<(FStream& s, FInfluenceWeights& w);
};

struct FInfluenceBones
{
	union
	{
		struct
		{
			uint8 InfluenceBones[MAX_INFLUENCES];
		};
		uint32 InfluenceBonesUint32 = 0;
	};

	friend FStream& operator<<(FStream& s, FInfluenceBones& w);
};

struct FVertexInfluence {
	FInfluenceWeights Wieghts;
	FInfluenceBones Bones;

	friend FStream& operator<<(FStream& s, FVertexInfluence& i);
};

enum EInstanceWeightUsage : uint8
{
	IWU_PartialSwap = 0,
	IWU_FullSwap
};

struct FSkeletalMeshVertexInfluences {
	std::vector<FVertexInfluence> Influences;
	std::map<FBoneIndexPair, std::vector<uint32>> VertexInfluenceMap;
	std::vector<FSkelMeshSection> Sections;
	std::vector<FSkelMeshChunk> Chunks;
	std::vector<uint8> RequiredBones;
	EInstanceWeightUsage Usage = IWU_PartialSwap;

	friend FStream& operator<<(FStream& s, FSkeletalMeshVertexInfluences& i);
};

class FStaticLODModel {
public:

	void Serialize(FStream& s, UObject* owner);

	std::vector<FSkelMeshSection> Sections;
	std::vector<uint16> LegacyShadowIndices;
	std::vector<uint8> LegacyShadowTriangleDoubleSided;
	std::vector<FSkelMeshChunk> Chunks;
	std::vector<uint16> ActiveBoneIndices;
	std::vector<uint8> RequiredBones;
	std::vector<FMeshEdge> LegacyEdges;
	FMultiSizeIndexContainer IndexContainer;
	uint32 Size = 0;
	uint32 NumVertices = 0;
	uint32 NumTexCoords = 1;

	FIntBulkData RawPointIndices;
	FWordBulkData LegacyRawPointIndices;

	FSkeletalMeshVertexBuffer VertexBufferGPUSkin;
	FSkeletalMeshColorBuffer ColorBuffer;
	std::vector<FSkeletalMeshVertexInfluences> VertexInfluences;
};

class USkeletalMesh : public UObject {
public:
  DECL_UOBJ(USkeletalMesh, UObject);

	UPROP(bool, bHasVertexColors, false);

	void Serialize(FStream& s) override;

private:
  FBoxSphereBounds Bounds;
  FVector Origin;
  FRotator RotOrigin;

	std::vector<FMeshBone> RefSkeleton;
	int32 SkeletalDepth = 0;

	std::vector<UObject*> Materials;
	std::vector<UObject*> ClothingAssets;

	std::vector<FStaticLODModel> LodModels;
	std::map<FName, int32> NameIndexMap;
};