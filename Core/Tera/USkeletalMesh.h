#pragma once
#include "UObject.h"

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
			uint16 ntris;
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

	int32 MaxBoneInfluences = 0;

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
};

template<uint32 NumTexCoords = 1>
struct TGPUSkinVertexFloat16Uvs : public FGPUSkinVertexBase {
	FVector Position;
	FVector2DHalf UV[NumTexCoords];
};

struct  FSkeletalMeshVertexBuffer {
	uint32 NumVertices = 0;
	uint32 NumTexCoords = 1;
	bool bUseFullPrecisionUVs = false;
	bool bUsePackedPosition = false;
	FVector MeshOrigin;
	FVector MeshExtension;

	friend FStream& operator<<(FStream& s, FSkeletalMeshVertexBuffer& b);
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
};

class USkeletalMesh : public UObject {
public:
  DECL_UOBJ(USkeletalMesh, UObject);

	UPROP(bool, bHasVertexColors, false);

	void Serialize(FStream& s);

private:
  FBoxSphereBounds Bounds;
  FVector Origin;
  FRotator RotOrigin;

	std::vector<FMeshBone> RefSkeleton;
	int32 SkeletalDepth = 0;

	std::vector<UObject*> Materials;
	std::vector<UObject*> ClothingAssets;
};