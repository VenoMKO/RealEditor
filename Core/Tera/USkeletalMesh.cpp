#include "USkeletalMesh.h"
#include "FObjectResource.h"

FStream& operator<<(FStream& s, FRigidSkinVertex& v)
{
  s << v.Position;
  s << v.TangentX;
  s << v.TangentY;
  s << v.TangentZ;

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    for (int32 idx = 0; idx < MAX_TEXCOORDS; ++idx)
    {
      s << v.UVs[idx];
    }
  }
  else
  {
    s << v.UVs[0].X << v.UVs[0].Y;
  }

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << v.Color;
  }

  s << v.Bone;
  return s;
}

FStream& operator<<(FStream& s, FSoftSkinVertex& v)
{
  s << v.Position;
  s << v.TangentX;
  s << v.TangentY;
  s << v.TangentZ;

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    for (int32 idx = 0; idx < MAX_TEXCOORDS; ++idx)
    {
      s << v.UVs[idx];
    }
  }
  else
  {
    s << v.UVs[0].X << v.UVs[0].Y;
  }

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << v.Color;
  }

  for (uint32 idx = 0; idx < MAX_INFLUENCES; idx++)
  {
    s << v.InfluenceBones[idx];
  }

  for (uint32 idx = 0; idx < MAX_INFLUENCES; idx++)
  {
    s << v.InfluenceWeights[idx];
  }

  return s;
}

FStream& operator<<(FStream& s, FSkelMeshChunk& c)
{
  s << c.BaseVertexIndex;
  s << c.RigidVertices;
  s << c.SoftVertices;
  s << c.BoneMap;
  s << c.NumRigidVertices;
  if (c.RigidVertices.size() != c.NumRigidVertices)
  {
    UThrow("Failed to serialize a mesh chunk. RV mismatch: %zu != %d", c.RigidVertices.size(), c.NumRigidVertices);
  }
  s << c.NumSoftVertices;
  if (c.SoftVertices.size() != c.NumSoftVertices)
  {
    UThrow("Failed to serialize a mesh chunk. SV mismatch: %zu != %d", c.SoftVertices.size(), c.NumSoftVertices);
  }
  s << c.MaxBoneInfluences;
  if (s.IsReading())
  {
    for (FRigidSkinVertex& v : c.RigidVertices)
    {
      v.BoneMap = &c.BoneMap;
    }
    for (FSoftSkinVertex& v : c.SoftVertices)
    {
      v.BoneMap = &c.BoneMap;
    }
  }
  return s;
}

FStream& operator<<(FStream& s, FMeshEdge& e)
{
  return s << e.Vertices[0] << e.Vertices[1] << e.Faces[0] << e.Faces[1];
}

FStream& operator<<(FStream& s, FSkeletalMeshVertexBuffer& b)
{
#define ALLOCATE_VERTEX_DATA_TEMPLATE( VertexDataType, numUVs ) \
  switch(numUVs) \
  { \
    case 1: b.Data = (FGPUSkinVertexBase*)new VertexDataType<1>[b.ElementCount]; break; \
    case 2: b.Data = (FGPUSkinVertexBase*)new VertexDataType<2>[b.ElementCount]; break; \
    case 3: b.Data = (FGPUSkinVertexBase*)new VertexDataType<3>[b.ElementCount]; break; \
    case 4: b.Data = (FGPUSkinVertexBase*)new VertexDataType<4>[b.ElementCount]; break; \
  }

#define SERIALIZE_VERTEX_DATA_TEMPLATE( VertexDataType, numUVs, idx ) \
  switch(numUVs) \
  { \
    case 1: s << ((VertexDataType<1>*)b.Data)[idx]; break; \
    case 2: s << ((VertexDataType<2>*)b.Data)[idx]; break; \
    case 3: s << ((VertexDataType<3>*)b.Data)[idx]; break; \
    case 4: s << ((VertexDataType<4>*)b.Data)[idx]; break; \
   }

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << b.NumTexCoords;
  }

  s << b.bUseFullPrecisionUVs;
  s << b.bUsePackedPosition;
  

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << b.MeshExtension;
    s << b.MeshOrigin;
  }

  s << b.ElementSize;
  s << b.ElementCount;

  if (b.ElementCount && b.ElementSize)
  {
    FILE_OFFSET len = s.GetPosition();
#if ENABLE_PACKED_VERTEX_POSITION
    if (!b.bUseFullPrecisionUVs)
    {
      if (b.bUsePackedPosition)
      {
        ALLOCATE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAB, b.NumTexCoords);
        
        FILE_OFFSET tmp = 0;
        for (uint32 idx = 0; idx < b.ElementCount; ++idx)
        {
          tmp = s.GetPosition();
          SERIALIZE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAB, b.NumTexCoords, idx);
          tmp = s.GetPosition() - tmp;
          int x = 1;
        }
      }
      else
      {
        ALLOCATE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatABB, b.NumTexCoords);
        for (uint32 idx = 0; idx < b.ElementCount; ++idx)
        {
          SERIALIZE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatABB, b.NumTexCoords, idx);
        }
      }
    }
    else
    {
      if (b.bUsePackedPosition)
      {
        ALLOCATE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAAB, b.NumTexCoords);
        for (uint32 idx = 0; idx < b.ElementCount; ++idx)
        {
          SERIALIZE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAAB, b.NumTexCoords, idx);
        }
      }
      else
      {
        ALLOCATE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAABB, b.NumTexCoords);
        for (uint32 idx = 0; idx < b.ElementCount; ++idx)
        {
          SERIALIZE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAABB, b.NumTexCoords, idx);
        }
      }
    }
#else
    if (!b.bUseFullPrecisionUVs)
    {
      ALLOCATE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatABB, b.NumTexCoords);
      for (uint32 idx = 0; idx < b.ElementCount; ++idx)
      {
        SERIALIZE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatABB, b.NumTexCoords, idx);
      }
    }
    else
    {
      ALLOCATE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAABB, b.NumTexCoords);
      for (uint32 idx = 0; idx < b.ElementCount; ++idx)
      {
        SERIALIZE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAABB, b.NumTexCoords, idx);
      }
    }
#endif
    len = s.GetPosition() - len;
    if (len != b.ElementCount * b.ElementSize)
    {
      UThrow("Failed to serialize GPU vertex buffer. Size mismatch: %d != %d(P:%d,T:%d)", len, b.ElementCount * b.ElementSize, b.bUsePackedPosition, b.bUseFullPrecisionUVs);
    }
  }
  return s;
}

FStream& operator<<(FStream& s, FSkeletalMeshColorBuffer& b)
{
  s << b.ElementSize;
  s << b.ElementCount;

  if (b.ElementSize && b.ElementCount)
  {
    if (s.IsReading())
    {
      b.Data = new FColor[b.ElementCount];
    }
    FILE_OFFSET len = s.GetPosition();
    for (int32 idx = 0; idx < b.ElementCount; ++idx)
    {
      s << b.Data[idx];
    }
    len = s.GetPosition() - len;
    if (len != b.ElementCount * b.ElementSize)
    {
      UThrow("Failed to serialize GPU color buffer. Size mismatch: %d != %d", len, b.ElementCount * b.ElementSize);
    }
  }

  return s;
}

FStream& operator<<(FStream& s, FInfluenceWeights& w)
{
  return s << w.InfluenceWeightsUint32;
}

FStream& operator<<(FStream& s, FInfluenceBones& w)
{
  return s << w.InfluenceBonesUint32;
}

FStream& operator<<(FStream& s, FVertexInfluence& i)
{
  return s << i.Wieghts << i.Bones;
}

FStream& operator<<(FStream& s, FSkeletalMeshVertexInfluences& i)
{
  s << i.Influences;
  s << i.VertexInfluenceMap;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << i.Sections;
    s << i.Chunks;
    s << i.RequiredBones;
    uint8 usage = i.Usage;
    s << usage;
    if (s.IsReading())
    {
      i.Usage = (EInstanceWeightUsage)usage;
    }
  }
  return s;
}

FStream& operator<<(FStream& s, FPerPolyBoneCollisionData& d)
{
  return s << d.KDOPTree << d.Vertices;
}

void FStaticLODModel::Serialize(FStream& s, UObject* owner)
{
  s << Sections;
  s << IndexContainer;

  if (s.GetFV() == VER_TERA_CLASSIC)
  {
    s << LegacyShadowIndices;
  }

  s << ActiveBoneIndices;

  if (s.GetFV() == VER_TERA_CLASSIC)
  {
    s << LegacyShadowTriangleDoubleSided;
  }

  s << Chunks;
  s << Size;
  s << NumVertices;

  if (s.GetFV() == VER_TERA_CLASSIC)
  {
    s << LegacyEdges;
  }

  s << RequiredBones;

  if (s.GetFV() == VER_TERA_CLASSIC)
  {
    LegacyRawPointIndices.Serialize(s, owner);
    RawPointIndices.Realloc(LegacyRawPointIndices.GetElementCount());
    for (int32 idx = 0; idx < LegacyRawPointIndices.GetElementCount(); ++idx)
    {
      int32 v = *(uint16*)LegacyRawPointIndices.GetAllocation() + (idx * LegacyRawPointIndices.GetElementSize());
      memcpy((uint8*)RawPointIndices.GetAllocation() + (idx * RawPointIndices.GetElementSize()), &v, RawPointIndices.GetElementSize());
    }
  }
  else
  {
    RawPointIndices.Serialize(s, owner);
  }

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << NumTexCoords;
  }

  s << VertexBufferGPUSkin;

  if (s.GetFV() > VER_TERA_CLASSIC && ((USkeletalMesh*)owner)->bHasVertexColors)
  {
    s << ColorBuffer;
  }
  s << VertexInfluences;
  s << Unk;
  DBreakIf(Unk.GetElementCount());
}

std::vector<FSoftSkinVertex> FStaticLODModel::GetVertices() const
{
  std::vector<FSoftSkinVertex> result;
  for (const FSkelMeshChunk& chunk : Chunks)
  {
    for (const FRigidSkinVertex& vert : chunk.RigidVertices)
    {
      result.emplace_back(vert);
    }
    for (const FSoftSkinVertex& vert : chunk.SoftVertices)
    {
      result.emplace_back(vert);
    }
  }
  return result;
}

void FGPUSkinVertexBase::Serialize(FStream& s)
{
  s << TangentX;
  s << TangentZ;

  for (int32 idx = 0; idx < MAX_INFLUENCES; ++idx)
  {
    s << BoneIndex[idx];
  }
  for (int32 idx = 0; idx < MAX_INFLUENCES; ++idx)
  {
    s << BoneWeight[idx];
  }
}

bool USkeletalMesh::RegisterProperty(FPropertyTag* property)
{
  if (PROP_IS(property, bHasVertexColors))
  {
    bHasVertexColors = property->BoolVal;
    bHasVertexColorsProperty = property;
    return true;
  }
  return false;
}

void USkeletalMesh::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Bounds;
  s << Materials;
  s << Origin;
  s << RotOrigin;
  s << RefSkeleton;
  s << SkeletalDepth;
  int32 modelsCount = (int32)LodModels.size();
  s << modelsCount;
  if (s.IsReading())
  {
    LodModels.resize(modelsCount);
  }
  for (int32 idx = 0; idx < modelsCount; ++idx)
  {
    LodModels[idx].Serialize(s, this);
  }

  s << NameIndexMap;

#if _DEBUG
  // Untested code
  // TODO: remove the whole "#if _DEBUG ... #endif" block after UStaticMesh is implemented
  if (s.IsReading())
  {
    FILE_OFFSET tmp = s.GetPosition();
    int32 cnt = 0;
    s << cnt;
    DBreakIf(cnt); 
    s.SetPosition(tmp);
  }
#endif

  if (s.GetFV() == VER_TERA_CLASSIC)
  {
    // TODO: implement kDOPs for 32-bit client and remove this check
    return;
  }

  s << PerPolyBoneKDOPs;

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << BoneBreakNames;
    s << BoneBreakOptions;
    s << ApexClothing;
    s << CachedStreamingTextureFactors;
    s << Unk1;
    DBreakIf(Unk1 || (Export->SerialOffset + Export->SerialSize) - s.GetPosition());
  }
}