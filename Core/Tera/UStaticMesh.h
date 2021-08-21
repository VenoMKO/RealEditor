#pragma once
#include "UObject.h"
#include "FStream.h"
#include "FStructs.h"
#include "kDOP.h"
#include "ULightMap.h"

#include "UActorComponent.h"

struct FStaticMeshTriangle
{
  FVector Vertices[3];
  FVector2D UVs[3][8];
  FColor Colors[3];
  int32 MaterialIndex = 0;
  int32 FragmentIndex = 0;
  uint32 SmoothingMask = 0;
  int32 NumUVs = 0;

  FVector TangentX[3]; // Tangent
  FVector TangentY[3]; // Binormal
  FVector TangentZ[3]; // Normal

  // int to match memory alignment
  int32 bOverrideTangentBasis = false;
  int32 bExplicitNormals = false;
};

struct FStaticVertex
{
  FVector Position;
  FVector	TangentX;	// Tangent
  FVector TangentY;	// Binormal
  FVector TangentZ;	// Normal
  FVector2D UVs[MAX_TEXCOORDS];
  int32 NumUVs = 0;
  FColor Color;
};

struct FLegacyStaticMeshVertexBase
{
  FPackedNormal TangentX;
  FPackedNormal TangentZ;
  FColor Color;

  void Serialize(FStream& s)
  {
    s << TangentX;
    s << TangentZ;
    s << Color;
  }
};

template <uint32 NumTexCoords = 1>
struct FLegacyStaticMeshVertexA : public FLegacyStaticMeshVertexBase {
  FLegacyStaticMeshVertexA()
  {
    for (uint32 idx = 0; idx < NumTexCoords; ++idx)
    {
      UV[idx].X = 0;
      UV[idx].Y = 0;
    }
  }

  friend FStream& operator<<(FStream& s, FLegacyStaticMeshVertexA& v)
  {
    v.Serialize(s);
    for (uint32 idx = 0; idx < NumTexCoords; ++idx)
    {
      s << v.UV[idx];
    }
    return s;
  }

  FVector2DHalf UV[NumTexCoords];
};

template <uint32 NumTexCoords = 1>
struct FLegacyStaticMeshVertexAA : public FLegacyStaticMeshVertexBase {
  FLegacyStaticMeshVertexAA()
  {
    for (uint32 idx = 0; idx < NumTexCoords; ++idx)
    {
      UV[idx].X = 0;
      UV[idx].Y = 0;
    }
  }

  friend FStream& operator<<(FStream& s, FLegacyStaticMeshVertexAA& v)
  {
    v.Serialize(s);
    for (uint32 idx = 0; idx < NumTexCoords; ++idx)
    {
      s << v.UV[idx];
    }
    return s;
  }

  FVector2D UV[NumTexCoords];
};

struct FStaticMeshVertexBase {
  FPackedNormal TangentX; // Tangent
  FPackedNormal TangentZ; // Normal

  void Serialize(FStream& s)
  {
    s << TangentX;
    s << TangentZ;
  }

  FVector GetTangentX() const
  {
    return TangentX;
  }

  FVector GetTangentY() const
  {
    return (FVector(TangentZ) ^ FVector(TangentX)) * ((float)TangentZ.Vector.W / 127.5f - 1.0f);
  }

  FVector GetTangentZ() const
  {
    return TangentZ;
  }
};

template <uint32 NumTexCoords = 1>
struct FStaticMeshVertexA : public FStaticMeshVertexBase {

  FStaticMeshVertexA()
  {
    for (uint32 idx = 0; idx < NumTexCoords; ++idx)
    {
      UV[idx].X = 0;
      UV[idx].Y = 0;
    }
  }

  friend FStream& operator<<(FStream& s, FStaticMeshVertexA& v)
  {
    v.Serialize(s);
    for (uint32 idx = 0; idx < NumTexCoords; ++idx)
    {
      s << v.UV[idx];
    }
    return s;
  }

  FVector2DHalf UV[NumTexCoords];
};

template<uint32 NumTexCoords = 1>
struct FStaticMeshVertexAA : public FStaticMeshVertexBase {

  FStaticMeshVertexAA()
  {
    for (uint32 idx = 0; idx < NumTexCoords; ++idx)
    {
      UV[idx].X = 0;
      UV[idx].Y = 0;
    }
  }

  friend FStream& operator<<(FStream& s, FStaticMeshVertexAA& v)
  {
    v.Serialize(s);
    for (uint32 idx = 0; idx < NumTexCoords; ++idx)
    {
      s << v.UV[idx];
    }
    return s;
  }

  FVector2D UV[NumTexCoords];
};

struct FStaticMeshLegacyUnkBuffer {
  uint32 Stride = 0;
  uint32 VertexCount = 0;
  uint32 ElementSize = 0;
  uint32 ElementCount = 0;

  friend FStream& operator<<(FStream& s, FStaticMeshLegacyUnkBuffer& b);

  ~FStaticMeshLegacyUnkBuffer()
  {
    free(Data);
  }
  void* Data = nullptr;
};

struct FStaticMeshLegacyVertexBuffer {
  uint32 NumTexCoords = 1;
  uint32 Stride = 0;
  uint32 NumVertices = 0;
  bool bUseFullPrecisionUVs = false;
  uint32 ElementSize = 0;
  uint32 ElementCount = 0;
  FLegacyStaticMeshVertexBase* Data = nullptr;

  FPackedNormal& GetTangentX(int32 idx);
  FPackedNormal& GetTangentZ(int32 idx);
  FPackedNormal GetTangentX(int32 idx) const;
  FPackedNormal GetTangentZ(int32 idx) const;

  FVector2D GetUVs(int32 vertexIndex, int32 uvIndex) const;

  ~FStaticMeshLegacyVertexBuffer();

  const FLegacyStaticMeshVertexBase* GetVertex(int32 idx) const;

  friend FStream& operator<<(FStream& s, FStaticMeshLegacyVertexBuffer& b);
};

struct FLegacyShadowVolumeBuffer {
  uint32 ElementSize = 0;
  uint32 ElementCount = 0;

  ~FLegacyShadowVolumeBuffer()
  {
    delete[] Data;
  }

  friend FStream& operator<<(FStream& s, FLegacyShadowVolumeBuffer& b);

  FEdge* Data = nullptr;
};

struct FStaticMeshVertexBuffer {
  uint32 NumTexCoords = 1;
  uint32 Stride = 0;
  uint32 NumVertices = 0;
  bool bUseFullPrecisionUVs = false;
  uint32 ElementSize = 0;
  uint32 ElementCount = 0;
  FStaticMeshVertexBase* Data = nullptr;

  FPackedNormal& GetTangentX(int32 idx);
  FPackedNormal& GetTangentZ(int32 idx);

  void SetUVs(int32 vertexIndex, int32 uvIndex, const FVector2D& uvs);
  FVector2D GetUVs(int32 vertexIndex, int32 uvIndex) const;

  FStaticMeshVertexBuffer() = default;
  void InitFromLegacy(const FStaticMeshLegacyVertexBuffer& lb);

  ~FStaticMeshVertexBuffer();

  const FStaticMeshVertexBase* GetVertex(int32 idx) const;

  friend FStream& operator<<(FStream& s, FStaticMeshVertexBuffer& b);
};

struct FStaticMeshPositionBuffer {
  uint32 Stride = 0;
  uint32 NumVertices = 0;
  uint32 ElementSize = 0;
  uint32 ElementCount = 0;

  FVector* Data = nullptr;

  ~FStaticMeshPositionBuffer();

  friend FStream& operator<<(FStream& s, FStaticMeshPositionBuffer& b);
};

struct FStaticMeshVertexColorBuffer {
  uint32 Stride = 0;
  uint32 NumVertices = 0;
  uint32 ElementSize = 0;
  uint32 ElementCount = 0;

  FColor* Data = nullptr;

  ~FStaticMeshVertexColorBuffer();

  friend FStream& operator<<(FStream& s, FStaticMeshVertexColorBuffer& b);
};

struct FStaticMeshTriangleBulkData : public FUntypedBulkData
{
  int32 GetElementSize() const override;
  void SerializeElement(FStream& s, void* data, int32 elementIndex) override;
  bool RequiresSingleElementSerialization(FStream& s) override;
};

struct FFragmentRange
{
  int32 BaseIndex = 0;
  int32 NumPrimitives = 0;

  friend FStream& operator<<(FStream& s, FFragmentRange& r)
  {
    return s << r.BaseIndex << r.NumPrimitives;
  }
};

class FStaticMeshElement
{
public:
  DECL_UREF(UObject, Material);
  bool EnableCollision = false;
  bool EnableCollisionOld = false;
  bool bEnableShadowCasting = true;
  uint32 FirstIndex = 0;
  uint32 NumTriangles = 0;
  uint32 MinVertexIndex = 0;
  uint32 MaxVertexIndex = 0;
  int32 MaterialIndex = 0;
  std::vector<FFragmentRange> Fragments;

  bool operator==(const FStaticMeshElement& a) const
  {
    return MaterialIndex == a.MaterialIndex;
  }

  friend FStream& operator<<(FStream& s, FStaticMeshElement& e)
  {
    s << e.Material;
    s << e.EnableCollision;
    s << e.EnableCollisionOld;
    s << e.bEnableShadowCasting;
    s << e.FirstIndex;
    s << e.NumTriangles;
    s << e.MinVertexIndex;
    s << e.MaxVertexIndex;
    s << e.MaterialIndex;
    s << e.Fragments;

    if (s.GetFV() > VER_TERA_CLASSIC)
    {
      uint8 loadPS3Data = 0;
      s << loadPS3Data;
      // PC packages should not have PS3 mesh data
      if (loadPS3Data)
      {
        UThrow("Can't load the mesh. Unexpected PS3 data!");
      }
    }

    return s;
  }
};

class UStaticMesh;
class FStaticMeshRenderData {
public:
  void Serialize(FStream& s, UObject* owner, int32 idx);

  std::vector<FStaticVertex> GetVertices() const;
  std::vector<FStaticMeshElement> GetElements() const
  {
    return Elements;
  }

  FStaticMeshTriangle* GetRawTriangles() const
  {
    return (FStaticMeshTriangle*)RawTriangles.BulkData;
  }

  int32 GetNumFaces() const
  {
    int32 result = 0;
    for (const FStaticMeshElement& e : Elements)
    {
      result += (int32)e.NumTriangles;
    }
    return result;
  }

  UStaticMesh* Owner = nullptr;

  FStaticMeshVertexBuffer VertexBuffer;
  FStaticMeshLegacyVertexBuffer LegacyVertexBuffer;
  FStaticMeshLegacyUnkBuffer LegacyUnkBuffer;
  FStaticMeshPositionBuffer PositionBuffer;
  FStaticMeshVertexColorBuffer ColorBuffer;
  FStaticMeshTriangleBulkData RawTriangles;
  std::vector<FStaticMeshElement> Elements;
  FLegacyShadowVolumeBuffer LegacyShadowVolumeEdges;
  FRawIndexBuffer IndexBuffer;
  FRawIndexBuffer WireframeIndexBuffer;
  FRawIndexBuffer Unk;
  uint32 LegacyUnk = 0;

  uint32 NumVertices = 0;
};

class UStaticMesh : public UObject {
public:
  DECL_UOBJ(UStaticMesh, UObject);
  // Construct StaticMesh class and link properties
  static void ConfigureClassObject(UClass* object);

  UPROP(bool, UseSimpleLineCollision, true);
  UPROP(bool, UseSimpleBoxCollision, true);

  ~UStaticMesh() override;
  bool RegisterProperty(FPropertyTag* property) override;
  void Serialize(FStream& s) override;

  const FStaticMeshRenderData* GetLod(int32 idx) const
  {
    if (idx >= LODModels.size())
    {
      return nullptr;
    }
    return &LODModels[idx];
  }

  int32 GetLodCount() const
  {
    return (int32)LODModels.size();
  }

  class URB_BodySetup* GetBodySetup() const;

  std::vector<UObject*> GetMaterials(int32 lodIdx = -1) const;

  const FBoxSphereBounds& GetBounds() const
  {
    return Bounds;
  }

protected:
  DECL_UREF(UObject, FBodySetup);

  FString Source;
  FBoxSphereBounds Bounds;
  
  FkDOPTreeCompact kDOPTree;
  FkDOPTree kDOPTreeLegacy;
  
  int32 SMDataVersion = 0;
  int32 VertexPositionVersionNumber = 0;
  
  int32 LodInfoCount = 0;
  
  FRotator ThumbnailAngle;
  float ThumbnailDistance = 0;

  FString HighResSourceMeshName;
  uint32 HighResSourceMeshCRC = 0;
  
  FGuid LightingGuid;
  
  bool bRemoveDegenerates = true;
  std::vector<FString> ContentTags;
  std::vector<float> CachedStreamingTextureFactors;
  std::vector<FStaticMeshRenderData> LODModels;
  std::vector<FVector> PhysMeshScale3D;

  FILE_OFFSET UnkSize = 8;
  void* Unk = nullptr;
  uint32 Unk2 = 0;
};

struct FStaticMeshComponentLODInfo {
  FObjectArray<UObject*> ShadowMaps;
  FObjectArray<UObject*> ShadowVertexBuffers;
  FLightMap* LightMap = nullptr;
  DECL_UREF(UObject*, Unk1);
  FStaticMeshVertexColorBuffer* OverrideVertexColors = nullptr;
  std::vector<FVector> VertexColorPositions;

  ~FStaticMeshComponentLODInfo()
  {
    if (LightMap)
    {
      delete LightMap;
    }
    if (OverrideVertexColors)
    {
      delete OverrideVertexColors;
    }
  }

  friend FStream& operator<<(FStream& s, FStaticMeshComponentLODInfo& i);
};

class UStaticMeshComponent : public UMeshComponent {
public:
  DECL_UOBJ(UStaticMeshComponent, UMeshComponent);

  bool RegisterProperty(FPropertyTag* property) override;

  UPROP(UStaticMesh*, StaticMesh, nullptr);

  void Serialize(FStream& s) override;
  void PostLoad() override;

  std::vector<FStaticMeshComponentLODInfo> LodData;
};