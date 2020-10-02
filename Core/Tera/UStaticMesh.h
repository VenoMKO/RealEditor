#pragma once
#include "UObject.h"
#include "FStream.h"
#include "FStructs.h"
#include "kDOP.h"

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

struct FStaticMeshVertexBase {
	FPackedNormal TangentX; // Tangent
	FPackedNormal TangentZ; // Normal

  virtual ~FStaticMeshVertexBase()
  {}

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

  virtual FVector2D GetUVs(int32 idx) const = 0;
};

template<uint32 NumTexCoords = 1>
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

  FVector2D GetUVs(int32 idx) const override
  {
    return UV[idx];
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

  FVector2D GetUVs(int32 idx) const override
  {
    return UV[idx];
  }

  FVector2D UV[NumTexCoords];
};


struct FStaticMeshVertexBuffer {
  uint32 NumTexCoords = 1;
  uint32 Stride = 0;
  uint32 NumVertices = 0;
  bool bUseFullPrecisionUVs = false;
  uint32 ElementSize = 0;
  uint32 ElementCount = 0;
  FStaticMeshVertexBase* Data = nullptr;

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

		if (s.GetFV() >= VER_TERA_CLASSIC)
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

  UStaticMesh* Owner = nullptr;

  FStaticMeshVertexBuffer VertexBuffer;
  FStaticMeshPositionBuffer PositionBuffer;
  FStaticMeshVertexColorBuffer ColorBuffer;
  FStaticMeshTriangleBulkData RawTriangles;
  std::vector<FStaticMeshElement> Elements;
  FRawIndexBuffer IndexBuffer;
  FRawIndexBuffer WireframeIndexBuffer;
  FRawIndexBuffer Unk;

  uint32 NumVertices = 0;
};

class UStaticMesh : public UObject {
public:
  DECL_UOBJ(UStaticMesh, UObject);
  // Construct StaticMesh class and link properties
  static void ConfigureClassObject(UClass* object);

  ~UStaticMesh() override;

  void Serialize(FStream& s) override;

  const FStaticMeshRenderData* GetLod(int32 idx) const
  {
    if (idx >= LODModels.size())
    {
      return nullptr;
    }
    return &LODModels[idx];
  }

  std::vector<UObject*> GetMaterials() const;

protected:
  DECL_UREF(UObject, FBodySetup);

  FString Source;
  FBoxSphereBounds Bounds;
  
  FkDOPTreeCompact kDOPTree;
  
  int32 SMDataVersion = 0;
  int32 VertexPositionVersionNumber = 0;
  
  int32 LodInfoCount = 0;
  
  FRotator ThumbnailAngle;
  float ThumbnailDistance = 0;

  FString HighResSourceMeshName;
  uint32 HighResSourceMeshCRC = 0;
  
  FGuid LightingGuid;
  
  bool bRemoveDegenerates = true;
  std::vector<float> CachedStreamingTextureFactors;
  std::vector<FStaticMeshRenderData> LODModels;

  FILE_OFFSET UnkSize = 8;
  void* Unk = nullptr;
};