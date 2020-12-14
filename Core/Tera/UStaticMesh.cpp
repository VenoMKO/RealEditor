#include "UStaticMesh.h"
#include "UClass.h"
#include "UProperty.h"
#include "Cast.h"

#include "FPackage.h"
#include "FObjectResource.h"
#include "UPhysAsset.h"

FStream& operator<<(FStream& s, FStaticMeshVertexBuffer& b)
{
#define ALLOCATE_VERTEX_DATA_TEMPLATE( VertexDataType, numUVs ) \
  switch(numUVs) \
  { \
    case 1: b.Data = (FStaticMeshVertexBase*)new VertexDataType<1>[b.ElementCount]; break; \
    case 2: b.Data = (FStaticMeshVertexBase*)new VertexDataType<2>[b.ElementCount]; break; \
    case 3: b.Data = (FStaticMeshVertexBase*)new VertexDataType<3>[b.ElementCount]; break; \
    case 4: b.Data = (FStaticMeshVertexBase*)new VertexDataType<4>[b.ElementCount]; break; \
  }
#define SERIALIZE_VERTEX_DATA_TEMPLATE( VertexDataType, numUVs, idx ) \
  switch(numUVs) \
  { \
    case 1: s << ((VertexDataType<1>*)b.Data)[idx]; break; \
    case 2: s << ((VertexDataType<2>*)b.Data)[idx]; break; \
    case 3: s << ((VertexDataType<3>*)b.Data)[idx]; break; \
    case 4: s << ((VertexDataType<4>*)b.Data)[idx]; break; \
   }

  s << b.NumTexCoords;
  s << b.Stride;
  s << b.NumVertices;
  s << b.bUseFullPrecisionUVs;

  if (b.Stride && b.NumVertices)
  {
    s << b.ElementSize;
    s << b.ElementCount;

    DBreakIf(b.Stride != b.ElementSize || b.NumVertices != b.ElementCount);

    if (s.IsReading())
    {
      if (!b.bUseFullPrecisionUVs)
      {
        ALLOCATE_VERTEX_DATA_TEMPLATE(FStaticMeshVertexA, b.NumTexCoords);
      }
      else
      {
        ALLOCATE_VERTEX_DATA_TEMPLATE(FStaticMeshVertexAA, b.NumTexCoords);
      }
    }

    if (!b.bUseFullPrecisionUVs)
    {
      for (int32 idx = 0; idx < b.ElementCount; ++idx)
      {
        SERIALIZE_VERTEX_DATA_TEMPLATE(FStaticMeshVertexA, b.NumTexCoords, idx);
      }
    }
    else
    {
      for (int32 idx = 0; idx < b.ElementCount; ++idx)
      {
        SERIALIZE_VERTEX_DATA_TEMPLATE(FStaticMeshVertexAA, b.NumTexCoords, idx);
      }
    }
  }
  return s;
}

FStream& operator<<(FStream& s, FStaticMeshPositionBuffer& b)
{
  s << b.Stride;
  s << b.NumVertices;

  if (b.Stride && b.NumVertices)
  {
    s << b.ElementSize;
    s << b.ElementCount;

    DBreakIf(b.Stride != b.ElementSize || b.NumVertices != b.ElementCount);

    if (s.IsReading())
    {
      b.Data = new FVector[b.ElementCount];
    }
    for (uint32 idx = 0; idx < b.ElementCount; ++idx)
    {
      s << b.Data[idx];
    }
  }
  return s;
}

FStream& operator<<(FStream& s, FStaticMeshVertexColorBuffer& b)
{
  s << b.Stride;
  s << b.NumVertices;

  if (b.Stride && b.NumVertices)
  {
    s << b.ElementSize;
    s << b.ElementCount;

    DBreakIf(b.Stride != b.ElementSize || b.NumVertices != b.ElementCount);

    if (s.IsReading())
    {
      b.Data = new FColor[b.ElementCount];
    }
    for (uint32 idx = 0; idx < b.ElementCount; ++idx)
    {
      s << b.Data[idx];
    }
  }
  return s;
}

void UStaticMesh::ConfigureClassObject(UClass* object)
{
  CreateProperty("UseSimpleLineCollision", UBoolProperty::StaticClassName(), object);
  CreateProperty("UseSimpleBoxCollision", UBoolProperty::StaticClassName(), object);
  CreateProperty("UseSimpleRigidBodyCollision", UBoolProperty::StaticClassName(), object);
  CreateProperty("UseFullPrecisionUVs", UBoolProperty::StaticClassName(), object);
  CreateProperty("bUsedForInstancing", UBoolProperty::StaticClassName(), object);
  CreateProperty("bUseMaximumStreamingTexelRatio", UBoolProperty::StaticClassName(), object);
  CreateProperty("bCanBecomeDynamic", UBoolProperty::StaticClassName(), object);

  CreateProperty("LightMapResolution", UIntProperty::StaticClassName(), object);
  CreateProperty("LightMapCoordinateIndex", UIntProperty::StaticClassName(), object);
  CreateProperty("LODDistanceRatio", UFloatProperty::StaticClassName(), object);
  CreateProperty("LODMaxRange", UFloatProperty::StaticClassName(), object);
  CreateProperty("StreamingDistanceMultiplier", UFloatProperty::StaticClassName(), object);

  CreateProperty("BodySetup", UObjectProperty::StaticClassName(), object);

  object->Link();
}

UStaticMesh::~UStaticMesh()
{
  free(Unk);
}

void FStaticMeshRenderData::Serialize(FStream& s, UObject* owner, int32 idx)
{
  if (s.IsReading())
  {
    Owner = (UStaticMesh*)owner;
  }
  RawTriangles.Serialize(s, owner);
  s << Elements;
  s << PositionBuffer;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << VertexBuffer;
    s << ColorBuffer;
  }
  else
  {
    // TODO
    UThrow("x86 Tera unsupported yet!");
  }
  s << NumVertices;
  s << IndexBuffer;
  s << WireframeIndexBuffer;

  if (s.GetFV() == VER_TERA_CLASSIC)
  {
    // TODO: legacy shadow volume edges
    UThrow("x86 Tera unsupported yet!");
  }

  s << Unk;
}

std::vector<FStaticVertex> FStaticMeshRenderData::GetVertices() const
{
  std::vector<FStaticVertex> result;
  result.resize(NumVertices);
  for (int32 idx = 0; idx < NumVertices; ++idx)
  {
    result[idx].Position = PositionBuffer.Data[idx];
    const FStaticMeshVertexBase* v = VertexBuffer.GetVertex(idx);
    result[idx].TangentX = v->GetTangentX();
    result[idx].TangentY = v->GetTangentY();
    result[idx].TangentZ = v->GetTangentZ();
    result[idx].NumUVs = VertexBuffer.NumTexCoords;
    result[idx].Color = ColorBuffer.ElementCount ? ColorBuffer.Data[idx] : FColor();
    
    for (int32 uvIdx = 0; uvIdx < VertexBuffer.NumTexCoords; ++uvIdx)
    {
      result[idx].UVs[uvIdx] = v->GetUVs(uvIdx);
    }
  }
  return result;
}

int32 FStaticMeshTriangleBulkData::GetElementSize() const
{
  return sizeof(FStaticMeshTriangle);
}

void FStaticMeshTriangleBulkData::SerializeElement(FStream& s, void* data, int32 elementIndex)
{
  FStaticMeshTriangle& t = *((FStaticMeshTriangle*)data + elementIndex);
  s << t.Vertices[0];
  s << t.Vertices[1];
  s << t.Vertices[2];

  for (int32 vIdx = 0; vIdx < 3; vIdx++)
  {
    for (int32 UVIndex = 0; UVIndex < 8; UVIndex++)
    {
      s << t.UVs[vIdx][UVIndex];
    }
  }

  s << t.Colors[0];
  s << t.Colors[1];
  s << t.Colors[2];
  s << t.MaterialIndex;
  s << t.FragmentIndex;
  s << t.SmoothingMask;
  s << t.NumUVs;

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << t.bExplicitNormals;
  }

  s << t.TangentX[0];
  s << t.TangentX[1];
  s << t.TangentX[2];
  s << t.TangentY[0];
  s << t.TangentY[1];
  s << t.TangentY[2];
  s << t.TangentZ[0];
  s << t.TangentZ[1];
  s << t.TangentZ[2];

  s << t.bOverrideTangentBasis;
}

bool FStaticMeshTriangleBulkData::RequiresSingleElementSerialization(FStream& s)
{
  return s.GetFV() < VER_TERA_MODERN;
}

void UStaticMesh::Serialize(FStream& s)
{
  Super::Serialize(s);
  
  s << Source;
  s << Bounds;
  s << FBodySetup;

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << kDOPTree;
  }
  else
  {
    UThrow("x86 Tera unsupported yet!");
  }
  
  s << SMDataVersion;

  for (int32 idx = 0; idx < 4; ++idx)
  {
    int32 unk = 0;
    s << unk;
  }
  
  int32 cnt = (int32)LODModels.size();
  s << cnt;
  if (s.IsReading())
  {
    LODModels.resize(cnt);
  }
  for (int32 idx = 0; idx < cnt; ++idx)
  {
    LODModels[idx].Serialize(s, this, idx);
  }
  
  s << LodInfoCount;
  s << ThumbnailAngle;
  s << ThumbnailDistance;

  if (SMDataVersion >= 18)
  {
    s << HighResSourceMeshName;
    s << HighResSourceMeshCRC;
  }

  s << LightingGuid;

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << VertexPositionVersionNumber;
    s << CachedStreamingTextureFactors;
    s << bRemoveDegenerates;
  }

  if (s.IsReading())
  {
    free(Unk);
    Unk = nullptr;
    UnkSize = Export->SerialOffset + Export->SerialSize - s.GetPosition();
    DBreakIf(UnkSize != 8);
#if _DEBUG
    if (UnkSize < 0)
    {
      UThrow("Out of bounds!");
    }
    else if (UnkSize)
    {
      Unk = malloc(UnkSize);
      s.SerializeBytes(Unk, UnkSize);
      uint8* test = (uint8*)Unk;
      for (int32 idx = 0; idx < UnkSize; ++idx)
      {
        DBreakIf(test[idx]);
      }
    }
#else
    if (UnkSize < 0)
    {
      return;
    }
    else if (UnkSize)
    {
      Unk = malloc(UnkSize);
      s.SerializeBytes(Unk, UnkSize);
    }
#endif
  }
  else if (Unk)
  {
    s.SerializeBytes(Unk, UnkSize);
  }
}

URB_BodySetup* UStaticMesh::GetBodySetup() const
{
  return Cast<URB_BodySetup>(FBodySetup);
}

std::vector<UObject*> UStaticMesh::GetMaterials() const
{
  std::vector<UObject*> result;
  for (const FStaticMeshRenderData& lod : LODModels)
  {
    std::vector<UObject*> lodMaterials;
    for (const FStaticMeshElement& element : lod.Elements)
    {
      lodMaterials.push_back(element.Material);
    }
    for (UObject* lodMaterial : lodMaterials)
    {
      for (UObject* material : result)
      {
        if (lodMaterial->GetPackage()->GetObjectIndex(lodMaterial) != material->GetPackage()->GetObjectIndex(material))
        {
          result.push_back(lodMaterial);
          break;
        }
      }
    }
  }
  return result;
}

FStaticMeshVertexBuffer::~FStaticMeshVertexBuffer()
{
  delete[] Data;
}

const FStaticMeshVertexBase* FStaticMeshVertexBuffer::GetVertex(int32 idx) const
{
  if (bUseFullPrecisionUVs)
  {
    switch (NumTexCoords)
    {
    case 1: return &((FStaticMeshVertexAA<1>*)Data)[idx];
    case 2: return &((FStaticMeshVertexAA<2>*)Data)[idx];
    case 3: return &((FStaticMeshVertexAA<3>*)Data)[idx];
    case 4: return &((FStaticMeshVertexAA<4>*)Data)[idx];
    }
  }
  else
  {
    switch (NumTexCoords)
    {
    case 1: return &((FStaticMeshVertexA<1>*)Data)[idx];
    case 2: return &((FStaticMeshVertexA<2>*)Data)[idx];
    case 3: return &((FStaticMeshVertexA<3>*)Data)[idx];
    case 4: return &((FStaticMeshVertexA<4>*)Data)[idx];
    }
  }
  return nullptr;
}

FStaticMeshPositionBuffer::~FStaticMeshPositionBuffer()
{
  delete[] Data;
}

FStaticMeshVertexColorBuffer::~FStaticMeshVertexColorBuffer()
{
  delete[] Data;
}

bool UStaticMeshComponent::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, StaticMesh))
  {
    StaticMeshProperty = property;
    StaticMesh = Cast<UStaticMesh>(GetPackage()->GetObject(property->Value->GetObjectIndex(), false));
    return true;
  }
  return false;
}

void UStaticMeshComponent::PostLoad()
{
  LoadObject(StaticMesh);
  Super::PostLoad();
}
