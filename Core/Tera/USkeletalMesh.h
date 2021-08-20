#pragma once
#include "UObject.h"
#include "kDOP.h"
#include <unordered_map>

#include "FStream.h"
#include "FStructs.h"
#include "FMeshCommon.h"
#include "FObjectArray.h"
#include "UActorComponent.h"
#include "UMaterial.h"

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

    if (s.GetFV() < VER_TERA_MODERN)
    {
      uint16 ntris = (uint16)m.NumTriangles;
      s << ntris;
      m.NumTriangles = ntris;
    }
    else
    {
      s << m.NumTriangles;
    }

    if (s.GetFV() > VER_TERA_CLASSIC || s.GetFV() == VER_BNS)
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
  std::vector<uint16> const* BoneMap = nullptr;

  friend FStream& operator<<(FStream& s, FRigidSkinVertex& v);
};

struct FSoftSkinVertex
{
  FSoftSkinVertex()
  {
    for (int32 idx = 0; idx < MAX_TEXCOORDS; ++idx)
    {
      UVs[idx].X = 0;
      UVs[idx].Y = 0;
    }
    for (int32 idx = 0; idx < MAX_INFLUENCES; ++idx)
    {
      InfluenceBones[idx] = 0;
      InfluenceWeights[idx] = 0;
    }
  }

  FSoftSkinVertex(const FRigidSkinVertex& v)
  {
    Position = v.Position;

    TangentX = v.TangentX;
    TangentY = v.TangentY;
    TangentZ = v.TangentZ;
    
    Color = v.Color;

    for (int32 idx = 0; idx < MAX_TEXCOORDS; ++idx)
    {
      UVs[idx] = v.UVs[idx];
    }

    BoneMap = v.BoneMap;

    InfluenceBones[0] = v.Bone;
    InfluenceWeights[0] = 0xFF;

    for (int32 idx = 1; idx < MAX_INFLUENCES; ++idx)
    {
      InfluenceBones[idx] = 0;
      InfluenceWeights[idx] = 0;
    }
  }

  FVector Position;
  FPackedNormal TangentX;	// Tangent
  FPackedNormal TangentY;	// Binormal
  FPackedNormal TangentZ;	// Normal
  FVector2D UVs[MAX_TEXCOORDS];
  FColor Color;
  uint8 InfluenceBones[MAX_INFLUENCES];
  uint8 InfluenceWeights[MAX_INFLUENCES];
  std::vector<uint16> const* BoneMap = nullptr;

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

  void CalcMaxBoneInfluences();

  friend FStream& operator<<(FStream& s, FSkelMeshChunk& c);
};

struct FGPUSkinVertexBase {
  FPackedNormal TangentX; // Tangent
  FPackedNormal TangentZ; // Normal
  uint8 BoneIndex[MAX_INFLUENCES];
  uint8 BoneWeight[MAX_INFLUENCES];

  void Serialize(FStream& s);
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
};

struct  FSkeletalMeshVertexBuffer {
  FSkeletalMeshVertexBuffer() = default;

  FSkeletalMeshVertexBuffer(const FSkeletalMeshVertexBuffer& a)
  {
    bUseFullPrecisionUVs = a.bUseFullPrecisionUVs;
    bUsePackedPosition = a.bUsePackedPosition;
    NumVertices = a.NumVertices;
    NumTexCoords = a.NumTexCoords;
    MeshOrigin = a.MeshOrigin;
    MeshExtension = a.MeshExtension;
    ElementSize = a.ElementSize;
    ElementCount = a.ElementCount;
    bDisableCompression = a.bDisableCompression;
    if (ElementCount && ElementSize)
    {
      AllocateBuffer();
      std::memcpy(Data, a.Data, ElementSize * ElementCount);
    }
  }

  FSkeletalMeshVertexBuffer& operator=(const FSkeletalMeshVertexBuffer& a)
  {
    bUseFullPrecisionUVs = a.bUseFullPrecisionUVs;
    bUsePackedPosition = a.bUsePackedPosition;
    NumVertices = a.NumVertices;
    NumTexCoords = a.NumTexCoords;
    MeshOrigin = a.MeshOrigin;
    MeshExtension = a.MeshExtension;
    ElementSize = a.ElementSize;
    ElementCount = a.ElementCount;
    bDisableCompression = a.bDisableCompression;
    if (ElementCount && ElementSize)
    {
      AllocateBuffer();
      std::memcpy(Data, a.Data, ElementSize * ElementCount);
    }
    return *this;
  }

  bool bDisableCompression = false;
  bool bUseFullPrecisionUVs = false;
  bool bUsePackedPosition = true;

  uint32 NumVertices = 0;
  uint32 NumTexCoords = 1;
  FVector MeshOrigin;
  FVector MeshExtension = FVector::One;
  uint32 ElementSize = 32;
  uint32 ElementCount = 0;
  FGPUSkinVertexBase* Data = nullptr;

  void AllocateBuffer(uint32 count = 0);
  void SetVertex(int32 vertexIndex, const FSoftSkinVertex& src, bool UVSpaceBinormalDir = true, bool flipBinormals = false);
  void GetVertex(int32 vertexIndex, FSoftSkinVertex& dst) const;

  friend FStream& operator<<(FStream& s, FSkeletalMeshVertexBuffer& b);
  
  ~FSkeletalMeshVertexBuffer()
  {
    delete[] Data;
    Data = nullptr;
  }
};

struct  FSkeletalMeshColorBuffer {
  FColor* Data = nullptr;
  uint32 ElementSize = 4;
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

struct FPerPolyBoneCollisionData {
  FkDOPTreeCompact KDOPTree;
  std::vector<FVector> Vertices;

  friend FStream& operator<<(FStream& s, FPerPolyBoneCollisionData& d);
};

class FStaticLODModel {
public:

  void Serialize(FStream& s, UObject* owner, int32 idx);

  std::vector<FSoftSkinVertex> GetVertices() const;
  // Buffer: true - GPU, false - CPU.
  std::vector<FSoftSkinVertex> GetVertices(bool gpuBuffer) const;

  std::vector<const FSkelMeshSection*> GetSections() const
  {
    std::vector<const FSkelMeshSection*> result;
    for (const auto& s : Sections)
    {
      result.emplace_back(&s);
    }
    return result;
  }

  int32 GetNumTexCoords() const
  {
    return NumTexCoords;
  }

  const FMultiSizeIndexContainer* GetIndexContainer() const
  {
    return &IndexContainer;
  }

  void _DebugVerify(UObject* owner) const;

private:
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
  FMultiSizeIndexContainer Unk;
  friend class USkeletalMesh;
};

class USkeletalMesh : public UObject {
public:
  DECL_UOBJ(USkeletalMesh, UObject);
  UPROP(bool, bHasVertexColors, false);
  UPROP(std::vector<FPropertyValue*>*, LODInfo, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;
  void Serialize(FStream& s) override;

  inline const FStaticLODModel* GetLod(int32 idx) const
  {
    if (idx >= LodModels.size())
    {
      return nullptr;
    }
    return &LodModels[idx];
  }

  inline std::vector<UObject*> GetMaterials() const
  {
    return Materials.GetUObjects();
  }

  std::vector<FMeshBone> GetReferenceSkeleton() const
  {
    return RefSkeleton;
  }

  bool AcceptVisitor(class MeshTravallerData* importData, uint32 lodIdx, FString& error);
  bool ValidateVisitor(class MeshTravallerData* importData, uint32 lodIdx, FString& error, bool& askUser, int32 warningIndex);

  const FBoxSphereBounds& GetBounds()
  {
    return Bounds;
  }

private:
  FBoxSphereBounds Bounds;
  FVector Origin;
  FRotator RotOrigin;

  std::vector<FMeshBone> RefSkeleton;
  int32 SkeletalDepth = 0;

  FObjectArray<UMaterialInterface*> Materials;
  FObjectArray<UObject*> ClothingAssets;

  std::vector<FStaticLODModel> LodModels;
  std::map<FName, int32> NameIndexMap;

  std::vector<FPerPolyBoneCollisionData> PerPolyBoneKDOPs;

  std::vector<FString> BoneBreakNames;
  std::vector<uint8> BoneBreakOptions;
  FObjectArray<UObject*> ApexClothing;
  std::vector<float> CachedStreamingTextureFactors;
  uint32 Unk1 = 0;
};

class USkeletalMeshComponent : public UMeshComponent {
public:
  DECL_UOBJ(USkeletalMeshComponent, UMeshComponent);

  bool RegisterProperty(FPropertyTag* property) override;

  UPROP(USkeletalMesh*, SkeletalMesh, nullptr);

  void PostLoad() override;
};