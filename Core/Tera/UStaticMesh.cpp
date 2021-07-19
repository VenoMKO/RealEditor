#include "UStaticMesh.h"
#include "UClass.h"
#include "UProperty.h"
#include "Cast.h"

#include "FPackage.h"
#include "FObjectResource.h"
#include "UPhysAsset.h"

FStream& operator<<(FStream& s, FStaticMeshVertexBuffer& b)
{
#define ALLOCATE_VERTEX_DATA_TEMPLATE( VertexDataType, numUVs, data, elementCount ) \
  switch(numUVs) \
  { \
    case 1: data = (FStaticMeshVertexBase*)new VertexDataType<1>[elementCount]; break; \
    case 2: data = (FStaticMeshVertexBase*)new VertexDataType<2>[elementCount]; break; \
    case 3: data = (FStaticMeshVertexBase*)new VertexDataType<3>[elementCount]; break; \
    case 4: data = (FStaticMeshVertexBase*)new VertexDataType<4>[elementCount]; break; \
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
        ALLOCATE_VERTEX_DATA_TEMPLATE(FStaticMeshVertexA, b.NumTexCoords, b.Data, b.ElementCount);
      }
      else
      {
        ALLOCATE_VERTEX_DATA_TEMPLATE(FStaticMeshVertexAA, b.NumTexCoords, b.Data, b.ElementCount);
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

FStream& operator<<(FStream& s, FStaticMeshComponentLODInfo& i)
{
  s << i.ShadowMaps;
  s << i.ShadowVertexBuffers;
  if (s.IsReading())
  {
    uint32 Type = FLightMap::LMT_None;
    s << Type;
    switch (Type)
    {
    case FLightMap::LMT_1D:
      i.LightMap = new FLightMap1D;
      break;
    case FLightMap::LMT_2D:
      i.LightMap = new FLightMap2D;
      break;
    }
    if (i.LightMap)
    {
      i.LightMap->Serialize(s);
    }
  }
  else
  {
    if (i.LightMap)
    {
      i.LightMap->Serialize(s);
    }
    else
    {
      uint32 Type = FLightMap::LMT_None;
      s << Type;
    }
  }

  SERIALIZE_UREF(s, i.Unk1);

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    uint8 bHasVertexColorOverride = i.OverrideVertexColors != nullptr;
    s << bHasVertexColorOverride;
    if (s.IsReading() && bHasVertexColorOverride)
    {
      i.OverrideVertexColors = new FStaticMeshVertexColorBuffer;
    }
    if (bHasVertexColorOverride)
    {
      s << *i.OverrideVertexColors;
    }
    s << i.VertexColorPositions;
  }
  return s;
}

FStream& operator<<(FStream& s, FStaticMeshLegacyVertexBuffer& b)
{
#define ALLOCATE_LEGACY_VERTEX_DATA_TEMPLATE( VertexDataType, numUVs, data, elementCount, elementSize ) \
  switch(numUVs) \
  { \
    case 1: data = (FLegacyStaticMeshVertexBase*)new VertexDataType<1>[elementCount]; DBreakIf(sizeof(VertexDataType<1>) != elementSize); break; \
    case 2: data = (FLegacyStaticMeshVertexBase*)new VertexDataType<2>[elementCount]; DBreakIf(sizeof(VertexDataType<2>) != elementSize); break; \
    case 3: data = (FLegacyStaticMeshVertexBase*)new VertexDataType<3>[elementCount]; DBreakIf(sizeof(VertexDataType<3>) != elementSize); break; \
    case 4: data = (FLegacyStaticMeshVertexBase*)new VertexDataType<4>[elementCount]; DBreakIf(sizeof(VertexDataType<4>) != elementSize); break; \
  }
#define SERIALIZE_LEGACY_VERTEX_DATA_TEMPLATE( VertexDataType, numUVs, idx ) \
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
        ALLOCATE_LEGACY_VERTEX_DATA_TEMPLATE(FLegacyStaticMeshVertexA, b.NumTexCoords, b.Data, b.ElementCount, b.ElementSize);
      }
      else
      {
        ALLOCATE_LEGACY_VERTEX_DATA_TEMPLATE(FLegacyStaticMeshVertexAA, b.NumTexCoords, b.Data, b.ElementCount, b.ElementSize);
      }
    }

    if (!b.bUseFullPrecisionUVs)
    {
      for (int32 idx = 0; idx < b.ElementCount; ++idx)
      {
        SERIALIZE_LEGACY_VERTEX_DATA_TEMPLATE(FLegacyStaticMeshVertexA, b.NumTexCoords, idx);
      }
    }
    else
    {
      for (int32 idx = 0; idx < b.ElementCount; ++idx)
      {
        SERIALIZE_LEGACY_VERTEX_DATA_TEMPLATE(FLegacyStaticMeshVertexAA, b.NumTexCoords, idx);
      }
    }
  }
  return s;
}

FStream& operator<<(FStream& s, FStaticMeshLegacyUnkBuffer& b)
{
  s << b.Stride;
  s << b.VertexCount;
  s << b.ElementSize;
  s << b.ElementCount;
  if (s.IsReading() && b.Stride && b.VertexCount)
  {
    b.Data = malloc(b.Stride * b.VertexCount);
  }
  s.SerializeBytes(b.Data, b.ElementCount * b.Stride);
  return s;
}

FStream& operator<<(FStream& s, FLegacyShadowVolumeBuffer& b)
{
  s << b.ElementSize;
  s << b.ElementCount;
  if (b.ElementCount * b.ElementSize)
  {
    DBreakIf(b.ElementSize != sizeof(FEdge));
    b.Data = new FEdge[b.ElementCount];
  }
  s.SerializeBytes(b.Data, b.ElementCount * b.ElementSize);
  return s;
}

void UStaticMesh::ConfigureClassObject(UClass* object)
{
  CreateClassProperty("UseSimpleLineCollision", UBoolProperty::StaticClassName(), object);
  CreateClassProperty("UseSimpleBoxCollision", UBoolProperty::StaticClassName(), object);
  CreateClassProperty("UseSimpleRigidBodyCollision", UBoolProperty::StaticClassName(), object);
  CreateClassProperty("UseFullPrecisionUVs", UBoolProperty::StaticClassName(), object);
  CreateClassProperty("bUsedForInstancing", UBoolProperty::StaticClassName(), object);
  CreateClassProperty("bUseMaximumStreamingTexelRatio", UBoolProperty::StaticClassName(), object);
  CreateClassProperty("bCanBecomeDynamic", UBoolProperty::StaticClassName(), object);

  CreateClassProperty("LightMapResolution", UIntProperty::StaticClassName(), object);
  CreateClassProperty("LightMapCoordinateIndex", UIntProperty::StaticClassName(), object);
  CreateClassProperty("LODDistanceRatio", UFloatProperty::StaticClassName(), object);
  CreateClassProperty("LODMaxRange", UFloatProperty::StaticClassName(), object);
  CreateClassProperty("StreamingDistanceMultiplier", UFloatProperty::StaticClassName(), object);

  CreateClassProperty("BodySetup", UObjectProperty::StaticClassName(), object);

  object->Link();
}

UStaticMesh::~UStaticMesh()
{
  free(Unk);
}

bool UStaticMesh::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(UseSimpleLineCollision);
  REGISTER_BOOL_PROP(UseSimpleBoxCollision);
  return false;
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
    s << LegacyVertexBuffer;
    s << LegacyUnkBuffer;
    VertexBuffer.InitFromLegacy(LegacyVertexBuffer);
  }
  s << NumVertices;
  s << IndexBuffer;
  s << WireframeIndexBuffer;

  if (s.GetFV() == VER_TERA_CLASSIC)
  {
    s << LegacyShadowVolumeEdges;
    s << LegacyUnk;
  }
  else
  {
    s << Unk;
  }
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
      result[idx].UVs[uvIdx] = VertexBuffer.GetUVs(idx, uvIdx);
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
    s << kDOPTreeLegacy;
  }
  
  s << SMDataVersion;

  if (s.GetFV() == VER_TERA_CLASSIC)
  {
    s << ContentTags;
  }
  else
  {
    for (int32 idx = 0; idx < 4; ++idx)
    {
      int32 unk = 0;
      s << unk;
    }
  }

  static int tcnt = 0;
  tcnt++;
  if (tcnt < 29)
  {
    return;
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

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    if (SMDataVersion >= 18)
    {
      s << HighResSourceMeshName;
      s << HighResSourceMeshCRC;
    }
    s << LightingGuid;
    s << VertexPositionVersionNumber;
    s << CachedStreamingTextureFactors;
    s << bRemoveDegenerates;
  }
  else
  {
    s << PhysMeshScale3D;
    s << Unk2;
  }

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
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
}

URB_BodySetup* UStaticMesh::GetBodySetup() const
{
  return Cast<URB_BodySetup>(FBodySetup);
}

std::vector<UObject*> UStaticMesh::GetMaterials(int32 lodIdx) const
{
  std::vector<UObject*> result;

  auto AddLodMaterials = [](const FStaticMeshRenderData& lod, std::vector<UObject*>& result) {
    std::vector<UObject*> lodMaterials;
    for (const FStaticMeshElement& element : lod.Elements)
    {
      if (element.NumTriangles)
      {
        lodMaterials.push_back(element.Material);
      }
    }
    for (UObject* lodMaterial : lodMaterials)
    {
      bool found = false;
      for (UObject* material : result)
      {
        if (lodMaterial == material || (lodMaterial && material && lodMaterial->GetPackage()->GetObjectIndex(lodMaterial) == material->GetPackage()->GetObjectIndex(material)))
        {
          found = true;
          break;
        }
      }
      if (!found)
      {
        result.push_back(lodMaterial);
      }
    }
  };

  if (lodIdx == -1 || lodIdx >= LODModels.size())
  {
    for (const FStaticMeshRenderData& lod : LODModels)
    {
      AddLodMaterials(lod, result);
    }
  }
  else
  {
    AddLodMaterials(LODModels[lodIdx], result);
  }
  return result;
}

FPackedNormal& FStaticMeshVertexBuffer::GetTangentX(int32 idx)
{
  if (!bUseFullPrecisionUVs)
  {
    return ((FStaticMeshVertexA<MAX_TEXCOORDS>*)((uint8*)Data + idx * Stride))->TangentX;
  }
  return ((FStaticMeshVertexAA<MAX_TEXCOORDS>*)((char*)Data + idx * Stride))->TangentX;
}

FPackedNormal& FStaticMeshVertexBuffer::GetTangentZ(int32 idx)
{
  if (!bUseFullPrecisionUVs)
  {
    return ((FStaticMeshVertexA<MAX_TEXCOORDS>*)((uint8*)Data + idx * Stride))->TangentZ;
  }
  return ((FStaticMeshVertexAA<MAX_TEXCOORDS>*)((char*)Data + idx * Stride))->TangentZ;
}

void FStaticMeshVertexBuffer::SetUVs(int32 vertexIndex, int32 uvIndex, const FVector2D& uvs)
{
  if (!bUseFullPrecisionUVs)
  {
    ((FStaticMeshVertexA<MAX_TEXCOORDS>*)((uint8*)Data + vertexIndex * Stride))->UV[uvIndex] = uvs;
  }
  else
  {
    ((FStaticMeshVertexAA<MAX_TEXCOORDS>*)((char*)Data + vertexIndex * Stride))->UV[uvIndex] = uvs;
  }
}

FVector2D FStaticMeshVertexBuffer::GetUVs(int32 vertexIndex, int32 uvIndex) const
{
  if (!bUseFullPrecisionUVs)
  {
    return ((FStaticMeshVertexA<MAX_TEXCOORDS>*)((char*)Data + vertexIndex * Stride))->UV[uvIndex];
  }
  return ((FStaticMeshVertexAA<MAX_TEXCOORDS>*)((char*)Data + vertexIndex * Stride))->UV[uvIndex];
}

void FStaticMeshVertexBuffer::InitFromLegacy(const FStaticMeshLegacyVertexBuffer& lb)
{
  NumTexCoords = lb.NumTexCoords;
  NumVertices = lb.NumVertices;
  bUseFullPrecisionUVs = lb.bUseFullPrecisionUVs;
  ElementCount = lb.ElementCount;

  if (!bUseFullPrecisionUVs)
  {
    ALLOCATE_VERTEX_DATA_TEMPLATE(FStaticMeshVertexA, NumTexCoords, Data, ElementCount);
    switch (NumTexCoords)
    {
    case 1: Stride = sizeof(FStaticMeshVertexA<1>); break;
    case 2: Stride = sizeof(FStaticMeshVertexA<2>); break;
    case 3: Stride = sizeof(FStaticMeshVertexA<3>); break;
    case 4: Stride = sizeof(FStaticMeshVertexA<4>); break;
    }
  }
  else
  {
    ALLOCATE_VERTEX_DATA_TEMPLATE(FStaticMeshVertexAA, NumTexCoords, Data, ElementCount);
    switch (NumTexCoords)
    {
    case 1: Stride = sizeof(FStaticMeshVertexAA<1>); break;
    case 2: Stride = sizeof(FStaticMeshVertexAA<2>); break;
    case 3: Stride = sizeof(FStaticMeshVertexAA<3>); break;
    case 4: Stride = sizeof(FStaticMeshVertexAA<4>); break;
    }
  }

  for (int32 idx = 0; idx < NumVertices; ++idx)
  {
    if (!bUseFullPrecisionUVs)
    {
      GetTangentX(idx) = lb.GetTangentX(idx);
      GetTangentZ(idx) = lb.GetTangentZ(idx);
      for (int32 uvIdx = 0; uvIdx < NumTexCoords; ++uvIdx)
      {
        SetUVs(idx, uvIdx, lb.GetUVs(idx, uvIdx));
      }
    }
  }
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

FPackedNormal& FStaticMeshLegacyVertexBuffer::GetTangentX(int32 idx)
{
  if (!bUseFullPrecisionUVs)
  {
    return ((FLegacyStaticMeshVertexA<MAX_TEXCOORDS>*)((uint8*)Data + idx * Stride))->TangentX;
  }
  return ((FLegacyStaticMeshVertexAA<MAX_TEXCOORDS>*)((uint8*)Data + idx * Stride))->TangentX;
}

FPackedNormal FStaticMeshLegacyVertexBuffer::GetTangentX(int32 idx) const
{
  if (!bUseFullPrecisionUVs)
  {
    return ((FLegacyStaticMeshVertexA<MAX_TEXCOORDS>*)((uint8*)Data + idx * Stride))->TangentX;
  }
  return ((FLegacyStaticMeshVertexAA<MAX_TEXCOORDS>*)((uint8*)Data + idx * Stride))->TangentX;
}

FPackedNormal& FStaticMeshLegacyVertexBuffer::GetTangentZ(int32 idx)
{
  if (!bUseFullPrecisionUVs)
  {
    return ((FLegacyStaticMeshVertexA<MAX_TEXCOORDS>*)((uint8*)Data + idx * Stride))->TangentZ;
  }
  return ((FLegacyStaticMeshVertexAA<MAX_TEXCOORDS>*)((uint8*)Data + idx * Stride))->TangentZ;
}

FPackedNormal FStaticMeshLegacyVertexBuffer::GetTangentZ(int32 idx) const
{
  if (!bUseFullPrecisionUVs)
  {
    return ((FLegacyStaticMeshVertexA<MAX_TEXCOORDS>*)((uint8*)Data + idx * Stride))->TangentZ;
  }
  return ((FLegacyStaticMeshVertexAA<MAX_TEXCOORDS>*)((uint8*)Data + idx * Stride))->TangentZ;
}

FVector2D FStaticMeshLegacyVertexBuffer::GetUVs(int32 vertexIndex, int32 uvIndex) const
{
  if (!bUseFullPrecisionUVs)
  {
    return ((FLegacyStaticMeshVertexA<MAX_TEXCOORDS>*)((uint8*)Data + vertexIndex * Stride))->UV[uvIndex];
  }
  return ((FLegacyStaticMeshVertexAA<MAX_TEXCOORDS>*)((uint8*)Data + vertexIndex * Stride))->UV[uvIndex];
}

FStaticMeshLegacyVertexBuffer::~FStaticMeshLegacyVertexBuffer()
{
  delete[] Data;
}

const FLegacyStaticMeshVertexBase* FStaticMeshLegacyVertexBuffer::GetVertex(int32 idx) const
{
  if (bUseFullPrecisionUVs)
  {
    switch (NumTexCoords)
    {
    case 1: return &((FLegacyStaticMeshVertexAA<1>*)Data)[idx];
    case 2: return &((FLegacyStaticMeshVertexAA<2>*)Data)[idx];
    case 3: return &((FLegacyStaticMeshVertexAA<3>*)Data)[idx];
    case 4: return &((FLegacyStaticMeshVertexAA<4>*)Data)[idx];
    }
  }
  else
  {
    switch (NumTexCoords)
    {
    case 1: return &((FLegacyStaticMeshVertexA<1>*)Data)[idx];
    case 2: return &((FLegacyStaticMeshVertexA<2>*)Data)[idx];
    case 3: return &((FLegacyStaticMeshVertexA<3>*)Data)[idx];
    case 4: return &((FLegacyStaticMeshVertexA<4>*)Data)[idx];
    }
  }
  return nullptr;
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

void UStaticMeshComponent::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << LodData;
}

void UStaticMeshComponent::PostLoad()
{
  LoadObject(StaticMesh);
  Super::PostLoad();
}
