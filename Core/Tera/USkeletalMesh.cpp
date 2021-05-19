#include "USkeletalMesh.h"
#include "FObjectResource.h"
#include <Tera/Cast.h>
#include <Utils/MeshTravaller.h>

#define SCALE_TOLERANCE .1f
#define THRESH_POINTS_ARE_SAME (0.002f)
#define THRESH_NORMALS_ARE_SAME (0.0002f)

struct FSkinVertexMeta {
  FSoftSkinVertex Vertex;
  uint32 PointWedgeIdx = 0;
};

bool PointsEqual(const FVector& V1, const FVector& V2, bool bNoEpsilon = false)
{
  const float e = bNoEpsilon ? 0.0f : THRESH_POINTS_ARE_SAME * 4.;
  if (abs(V1.X - V2.X) > e || abs(V1.Y - V2.Y) > e || abs(V1.Z - V2.Z) > e)
  {
    return false;
  }

  return true;
}

bool NormalsEqual(const FVector& V1, const FVector& V2)
{
  const float e = THRESH_NORMALS_ARE_SAME * 4.;
  if (abs(V1.X - V2.X) > e || abs(V1.Y - V2.Y) > e || abs(V1.Z - V2.Z) > e)
  {
    return false;
  }
  return true;
}

bool SkeletalMesh_UVsEqual(const RawWedge& V1, const RawWedge& V2, const int32 UVIndex = 0)
{
  const FVector2D& UV1 = V1.UV[UVIndex];
  const FVector2D& UV2 = V2.UV[UVIndex];

  if (abs(UV1.X - UV2.X) > (1.0f / 1024.0f))
    return 0;

  if (abs(UV1.Y - UV2.Y) > (1.0f / 1024.0f))
    return 0;

  return 1;
}

float GetBasisDeterminantSign(const FVector& XAxis, const FVector& YAxis, const FVector& ZAxis)
{
  FMatrix Basis(
    FPlane(XAxis, 0),
    FPlane(YAxis, 0),
    FPlane(ZAxis, 0),
    FPlane(0, 0, 0, 1)
  );
  return (Basis.Determinant() < 0) ? -1.0f : +1.0f;
}

uint8 GetBasisDeterminantSignByte(const FPackedNormal& XAxis, const FPackedNormal& YAxis, const FPackedNormal& ZAxis, bool flip)
{
  float sign = GetBasisDeterminantSign(XAxis, YAxis, ZAxis);
  if (flip)
  {
    sign *= -1.;
  }
  return Trunc(sign * 127.5f + 127.5f);
}

uint8 GetBasisDeterminantSignByte(const FVector2D& UV, bool flip)
{
  float sign = flip ? +1.0f : -1.0f;
  if (Trunc(ceil(UV.X)) % 2)
  {
    sign *= -1.;
  }
  return Trunc(sign * 127.5f + 127.5f);
}

int32 AddSkinVertex(std::vector<FSkinVertexMeta>& Vertices, FSkinVertexMeta& VertexMeta)
{
  FSoftSkinVertex& Vertex = VertexMeta.Vertex;

  for (uint32 VertexIndex = 0; VertexIndex < (uint32)Vertices.size(); VertexIndex++)
  {
    FSkinVertexMeta& OtherVertexMeta = Vertices[VertexIndex];
    FSoftSkinVertex& OtherVertex = OtherVertexMeta.Vertex;

    if (!PointsEqual(OtherVertex.Position, Vertex.Position))
      continue;

    bool bUVsEqual = true;
    for (int32 UVIdx = 0; UVIdx < MAX_TEXCOORDS; ++UVIdx)
    {
      if (abs(Vertex.UVs[UVIdx].X - OtherVertex.UVs[UVIdx].X) > (1.0f / 1024.0f))
      {
        bUVsEqual = false;
      };
      if (abs(Vertex.UVs[UVIdx].Y - OtherVertex.UVs[UVIdx].Y) > (1.0f / 1024.0f))
      {
        bUVsEqual = false;
      }
    }

    if (!bUVsEqual)
      continue;
    
    if (!NormalsEqual(OtherVertex.TangentX, Vertex.TangentX))
      continue;

    if (!NormalsEqual(OtherVertex.TangentY, Vertex.TangentY))
      continue;

    if (!NormalsEqual(OtherVertex.TangentZ, Vertex.TangentZ))
      continue;
    
    bool	InfluencesMatch = 1;
    for (uint32 InfluenceIndex = 0; InfluenceIndex < MAX_INFLUENCES; InfluenceIndex++)
    {
      if (Vertex.InfluenceBones[InfluenceIndex] != OtherVertex.InfluenceBones[InfluenceIndex] ||
          Vertex.InfluenceWeights[InfluenceIndex] != OtherVertex.InfluenceWeights[InfluenceIndex])
      {
        InfluencesMatch = 0;
        break;
      }
    }
    if (!InfluencesMatch)
      continue;

    return VertexIndex;
  }

  int32 result = Vertices.size();
  Vertices.emplace_back(VertexMeta);
  return result;
}

inline float FTriple(const FVector& X, const FVector& Y, const FVector& Z)
{
  return ((X.X * (Y.Y * Z.Z - Y.Z * Z.Y)) + (X.Y * (Y.Z * Z.X - Y.X * Z.Z)) + (X.Z * (Y.X * Z.Y - Y.Y * Z.X)));
}

template <typename TK, typename TV>
inline void MultiFindAll(const std::multimap<TK, TV>& mmap, TK key, std::vector<TV>& output)
{
  auto range = mmap.equal_range(key);
  for (auto i = range.first; i != range.second; ++i)
  {
    output.insert(output.begin(), i->second);
    //output.emplace_back(i->second);
  }
}

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
    if (!b.bUseFullPrecisionUVs)
    {
      if (b.bUsePackedPosition && b.bDisableCompression)
      {
        if (s.IsReading())
        {
          ALLOCATE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAB, b.NumTexCoords);
        }
        for (uint32 idx = 0; idx < b.ElementCount; ++idx)
        {
          SERIALIZE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAB, b.NumTexCoords, idx);
        }
      }
      else
      {
        if (s.IsReading())
        {
          ALLOCATE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatABB, b.NumTexCoords);
        }
        for (uint32 idx = 0; idx < b.ElementCount; ++idx)
        {
          SERIALIZE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatABB, b.NumTexCoords, idx);
        }
      }
    }
    else
    {
      if (b.bUsePackedPosition && b.bDisableCompression)
      {
        if (s.IsReading())
        {
          ALLOCATE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAAB, b.NumTexCoords);
        }
        for (uint32 idx = 0; idx < b.ElementCount; ++idx)
        {
          SERIALIZE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAAB, b.NumTexCoords, idx);
        }
      }
      else
      {
        if (s.IsReading())
        {
          ALLOCATE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAABB, b.NumTexCoords);
        }
        for (uint32 idx = 0; idx < b.ElementCount; ++idx)
        {
          SERIALIZE_VERTEX_DATA_TEMPLATE(FGPUSkinVertexFloatAABB, b.NumTexCoords, idx);
        }
      }
    }
    len = s.GetPosition() - len;
    if (len != b.ElementCount * b.ElementSize)
    {
      UThrow("Failed to serialize GPU vertex buffer. Size mismatch: %d != %d(P:%d,T:%d,C:%d)", len, b.ElementCount * b.ElementSize, b.bUsePackedPosition, b.bUseFullPrecisionUVs, b.bDisableCompression);
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

void FStaticLODModel::Serialize(FStream& s, UObject* owner, int32 idx)
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

  USkeletalMesh* mesh = Cast<USkeletalMesh>(owner);
  if (s.IsReading() && mesh->LODInfo && mesh->LODInfo->size() < idx)
  {
    FPropertyTag* disableComp = mesh->LODInfo->at(idx)->FindSubProperty("bDisableCompression");
    if (!disableComp)
    {
      disableComp = mesh->LODInfo->at(idx)->FindSubProperty("bDisableCompressions");
    }
    if (disableComp)
    {
      VertexBufferGPUSkin.bDisableCompression = disableComp->GetBool();
    }
  }

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
}

std::vector<FSoftSkinVertex> FStaticLODModel::GetVertices() const
{
  return GetVertices(USE_GPU_VERTEX_BUFFER);
}

std::vector<FSoftSkinVertex> FStaticLODModel::GetVertices(bool gpuBuffer) const
{
  std::vector<FSoftSkinVertex> result;
  result.reserve(NumVertices);
  if (gpuBuffer)
  {
    for (int32 idx = 0; idx < VertexBufferGPUSkin.ElementCount; ++idx)
    {
      FSoftSkinVertex& v = result.emplace_back();
      VertexBufferGPUSkin.GetVertex(idx, v);
    }
  }
  else
  {
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
  }
  return result;
}

void FStaticLODModel::_DebugVerify(UObject* owner) const
{
  USkeletalMesh* mesh = (USkeletalMesh*)owner;
  DBreakIf(Sections.empty());
  DBreakIf(Chunks.empty());

  const auto materials = mesh->GetMaterials();
  int32 totalNumTris = 0;
  for (const FSkelMeshSection& section : Sections)
  {
    DBreakIf(section.ChunkIndex >= Chunks.size());
    DBreakIf(section.MaterialIndex >= materials.size());
    DBreakIf(!section.NumTriangles);
    DBreakIf(section.BaseIndex > IndexContainer.GetElementCount());
    totalNumTris += section.NumTriangles;
  }
  
  int32 totalNumVerts = 0;
  const auto refSkel = mesh->GetReferenceSkeleton();
  for (const FSkelMeshChunk& chunk : Chunks)
  {
    totalNumVerts += chunk.NumRigidVertices + chunk.NumSoftVertices;
    DBreakIf(chunk.MaxBoneInfluences > 4 || !chunk.MaxBoneInfluences);
    DBreakIf(chunk.NumRigidVertices != chunk.RigidVertices.size());
    DBreakIf(chunk.NumSoftVertices != chunk.SoftVertices.size());
    DBreakIf(chunk.BoneMap.empty());
    for (const uint16& boneIdx : chunk.BoneMap)
    {
      DBreakIf(refSkel.size() <= boneIdx);
    }
  }
  DBreakIf(totalNumVerts != NumVertices);

  const auto cpuVerts = GetVertices(false);
  DBreakIf(cpuVerts.size() != VertexBufferGPUSkin.ElementCount);
  for (int32 idx = 0; idx < cpuVerts.size(); ++idx)
  {
    FSoftSkinVertex v;
    VertexBufferGPUSkin.GetVertex(idx, v);
    DBreakIf(!PointsEqual(cpuVerts[idx].Position, v.Position));
  }

  for (uint32 idx = 0; idx < IndexContainer.GetElementCount(); ++idx)
  {
    uint32 index = IndexContainer.GetIndex(idx);
    DBreakIf(index >= NumVertices);
  }
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
  if (PROP_IS(property, LODInfo))
  {
    LODInfo = &property->GetArray();
    LODInfoProperty = property;
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
    LodModels[idx].Serialize(s, this, idx);
  }

  s << NameIndexMap;

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
    DBreakIf(Unk1 || (s.IsReading() && (Export->SerialOffset + Export->SerialSize) - s.GetPosition()));
  }
}

struct FSkeletalMeshVertIndexAndZ {
  int32 Index = 0;
  float Z = 0.;
};

int SortSkeletalMeshVertIndexAndZ(const void* a, const void* b)
{
  const FSkeletalMeshVertIndexAndZ* A = (const FSkeletalMeshVertIndexAndZ*)a;
  const FSkeletalMeshVertIndexAndZ* B = (const FSkeletalMeshVertIndexAndZ*)b;
  if (A->Z < B->Z) return -1;
  if (A->Z > B->Z) return 1;
  return 0;
}

bool USkeletalMesh::AcceptVisitor(MeshTravallerData* importData, uint32 lodIdx, FString& error)
{
  std::vector<int32> matmap(importData->Materials.size());
  for (int32 idx = 0; idx < matmap.size(); ++idx)
  {
    auto name = importData->Materials[idx];
    auto it = std::find_if(importData->MaterialMap.begin(), importData->MaterialMap.end(), [&](const auto& p) { return p.first == name; });
    if (it != importData->MaterialMap.end())
    {
      matmap[idx] = std::find(importData->ObjectMaterials.begin(), importData->ObjectMaterials.end(), (*it).second) - importData->ObjectMaterials.begin();
    }
    else
    {
      matmap[idx] = 0;
    }
  }
  if (matmap.empty())
  {
    matmap.emplace_back(0);
  }

  for (auto& wedge : importData->Wedges)
  {
    wedge.materialIndex = matmap[wedge.materialIndex];
  }
  for (auto& face : importData->Faces)
  {
    face.materialIndex = matmap[face.materialIndex];
  }

  bool needs32bitIndexContainer = false;
  std::vector<FVector> faceTangentX(importData->Faces.size());
  std::vector<FVector> faceTangentY(importData->Faces.size());

  for (int32 faceIdx = 0; faceIdx < importData->Faces.size(); ++faceIdx)
  {
    FVector	p1 = importData->Points[importData->Wedges[importData->Faces[faceIdx].wedgeIndices[0]].pointIndex];
    FVector p2 = importData->Points[importData->Wedges[importData->Faces[faceIdx].wedgeIndices[1]].pointIndex];
    FVector p3 = importData->Points[importData->Wedges[importData->Faces[faceIdx].wedgeIndices[2]].pointIndex];
    FVector	triangleNormal = FPlane(p3, p2, p1);
    FMatrix	paramToLocal(
      FPlane(p2.X - p1.X, p2.Y - p1.Y, p2.Z - p1.Z, 0),
      FPlane(p3.X - p1.X, p3.Y - p1.Y, p3.Z - p1.Z, 0),
      FPlane(p1.X, p1.Y, p1.Z, 0),
      FPlane(0, 0, 0, 1)
    );
    float	U1 = importData->Wedges[importData->Faces[faceIdx].wedgeIndices[0]].UV[0].X,
          U2 = importData->Wedges[importData->Faces[faceIdx].wedgeIndices[1]].UV[0].X,
          U3 = importData->Wedges[importData->Faces[faceIdx].wedgeIndices[2]].UV[0].X,
          V1 = importData->Wedges[importData->Faces[faceIdx].wedgeIndices[0]].UV[0].Y,
          V2 = importData->Wedges[importData->Faces[faceIdx].wedgeIndices[1]].UV[0].Y,
          V3 = importData->Wedges[importData->Faces[faceIdx].wedgeIndices[2]].UV[0].Y;
    FMatrix	paramToTexture(
      FPlane(U2 - U1, V2 - V1, 0, 0),
      FPlane(U3 - U1, V3 - V1, 0, 0),
      FPlane(U1, V1, 1, 0),
      FPlane(0, 0, 0, 1)
    );
    FMatrix	texToLocal = paramToTexture.Inverse() * paramToLocal;
    FVector	tangentX = texToLocal.TransformNormal(FVector(1, 0, 0)).SafeNormal();
    FVector tangentY = texToLocal.TransformNormal(FVector(0, 1, 0)).SafeNormal();
    FVector tangentZ;

    tangentX = tangentX - triangleNormal * (tangentX | triangleNormal);
    tangentY = tangentY - triangleNormal * (tangentY | triangleNormal);
    faceTangentX[faceIdx] = tangentX.SafeNormal();
    faceTangentY[faceIdx] = tangentY.SafeNormal();
  }

  std::vector<int32> wedgeInfluenceIndices;
  std::map<uint32, uint32> vertexIndexToInfluenceIndexMap;
  for (uint32 lookIndex = 0; lookIndex < (uint32)importData->Influences.size(); lookIndex++)
  {
    if (!vertexIndexToInfluenceIndexMap.count(importData->Influences[lookIndex].vertexIndex))
    {
      vertexIndexToInfluenceIndexMap[importData->Influences[lookIndex].vertexIndex] = lookIndex;
    }
  }

  for (const auto& wedge : importData->Wedges)
  {
    wedgeInfluenceIndices.push_back(vertexIndexToInfluenceIndexMap[wedge.pointIndex]);
  }
  FStaticLODModel lod;
  std::multimap<int32, int32> vert2Duplicates;
  std::multimap<int32, int32> vert2Faces;
  {
    std::vector<FSkeletalMeshVertIndexAndZ> vertIndexAndZ;
    vertIndexAndZ.clear();
    vertIndexAndZ.reserve(importData->Points.size());
    for (int32 pointIndex = 0; pointIndex < importData->Points.size(); pointIndex++)
    {
      FSkeletalMeshVertIndexAndZ iandz;
      iandz.Index = pointIndex;
      iandz.Z = importData->Points[pointIndex].Z;
      vertIndexAndZ.push_back(iandz);
    }

    std::qsort(vertIndexAndZ.data(), vertIndexAndZ.size(), sizeof(decltype(vertIndexAndZ)::value_type), [](const void* a, const void* b)
    {
      const FSkeletalMeshVertIndexAndZ* A = (const FSkeletalMeshVertIndexAndZ*)a;
      const FSkeletalMeshVertIndexAndZ* B = (const FSkeletalMeshVertIndexAndZ*)b;
      if (A->Z < B->Z) return -1;
      if (A->Z > B->Z) return 1;
      return 0;
    });
    
    for (int32 i = 0; i < vertIndexAndZ.size(); i++)
    {
      for (int32 j = i + 1; j < vertIndexAndZ.size(); j++)
      {
        if (abs(vertIndexAndZ[j].Z - vertIndexAndZ[i].Z) > THRESH_POINTS_ARE_SAME)
        {
          break;
        }

        if (PointsEqual(importData->Points[vertIndexAndZ[i].Index], importData->Points[vertIndexAndZ[j].Index]))
        {
          vert2Duplicates.insert({ vertIndexAndZ[i].Index, vertIndexAndZ[j].Index });
          vert2Duplicates.insert({ vertIndexAndZ[j].Index, vertIndexAndZ[i].Index });
        }
      }
    }
    vertIndexAndZ.clear();

    for (int32 faceIndex = 0; faceIndex < importData->Faces.size(); faceIndex++)
    {
      const auto& face = importData->Faces[faceIndex];
      for (int32 vertexIndex = 0; vertexIndex < 3; vertexIndex++)
      {
        int32 key = importData->Wedges[face.wedgeIndices[vertexIndex]].pointIndex;
        if (std::find_if(vert2Faces.begin(), vert2Faces.end(), [=](const auto& p) { return p.first == key && p.second == faceIndex; }) == vert2Faces.end())
        {
          vert2Faces.emplace(importData->Wedges[face.wedgeIndices[vertexIndex]].pointIndex, faceIndex);
        }
      }
    }
  }

  std::vector<FRawIndexBuffer*> sectionIndexBufferArray;
  std::vector<std::vector<FSkinVertexMeta>*> chunkVerticesArray;
  std::vector<int32> adjacentFaces;
  std::vector<int32> dupVerts;
  std::vector<int32> dupFaces;

  for (int32 faceIndex = 0; faceIndex < importData->Faces.size(); faceIndex++)
  {
    const auto& face = importData->Faces[faceIndex];

    FVector	vertexTangentX[3];
    FVector vertexTangentY[3];
    FVector vertexTangentZ[3];

    for (int32 vertexIndex = 0; vertexIndex < 3; vertexIndex++)
    {
      vertexTangentX[vertexIndex] = FVector(0, 0, 0);
      vertexTangentY[vertexIndex] = FVector(0, 0, 0);
      vertexTangentZ[vertexIndex] = FVector(0, 0, 0);
    }

    FVector	triangleNormal = FPlane(
      importData->Points[importData->Wedges[face.wedgeIndices[2]].pointIndex],
      importData->Points[importData->Wedges[face.wedgeIndices[1]].pointIndex],
      importData->Points[importData->Wedges[face.wedgeIndices[0]].pointIndex]
    );
    float	determinant = FTriple(faceTangentX[faceIndex], faceTangentY[faceIndex], triangleNormal);

    adjacentFaces.clear();
    for (const auto& wedgeIndex : face.wedgeIndices)
    {
      int32 vert = importData->Wedges[wedgeIndex].pointIndex;
      dupVerts.clear();
      MultiFindAll(vert2Duplicates, vert, dupVerts);
      dupVerts.emplace_back(vert);
      for (const auto& dupVert : dupVerts)
      {
        dupFaces.clear();
        MultiFindAll(vert2Faces, dupVert, dupFaces);
        for (const auto& dupFace : dupFaces)
        {
          if (std::find(adjacentFaces.begin(), adjacentFaces.end(), dupFace) == adjacentFaces.end())
          {
            adjacentFaces.emplace_back(dupFace);
          }
        }
      }
    }

    for (int32 adjacentFaceIndex = 0; adjacentFaceIndex < adjacentFaces.size(); adjacentFaceIndex++)
    {
      int32 otherFaceIndex = adjacentFaces[adjacentFaceIndex];
      const auto& otherFace = importData->Faces[otherFaceIndex];
      FVector otherTriangleNormal = FPlane(
        importData->Points[importData->Wedges[otherFace.wedgeIndices[2]].pointIndex],
        importData->Points[importData->Wedges[otherFace.wedgeIndices[1]].pointIndex],
        importData->Points[importData->Wedges[otherFace.wedgeIndices[0]].pointIndex]
      );
      float otherFaceDeterminant = FTriple(faceTangentX[otherFaceIndex], faceTangentY[otherFaceIndex], otherTriangleNormal);

      for (int32 vertexIndex = 0; vertexIndex < 3; vertexIndex++)
      {
        for (int32 otherVertexIndex = 0; otherVertexIndex < 3; otherVertexIndex++)
        {
          if (PointsEqual(
            importData->Points[importData->Wedges[otherFace.wedgeIndices[otherVertexIndex]].pointIndex],
            importData->Points[importData->Wedges[face.wedgeIndices[vertexIndex]].pointIndex]
          ))
          {
            if (determinant * otherFaceDeterminant > 0.0f && SkeletalMesh_UVsEqual(importData->Wedges[otherFace.wedgeIndices[otherVertexIndex]], importData->Wedges[face.wedgeIndices[vertexIndex]]))
            {
              vertexTangentX[vertexIndex] += faceTangentX[otherFaceIndex];
              vertexTangentY[vertexIndex] += faceTangentY[otherFaceIndex];
            }
            if (importData->Wedges[otherFace.wedgeIndices[otherVertexIndex]].pointIndex == importData->Wedges[face.wedgeIndices[vertexIndex]].pointIndex)
            {
              vertexTangentZ[vertexIndex] += otherTriangleNormal;
            }
          }
        }
      }
    }

    FSkelMeshSection* section = nullptr;
    FSkelMeshChunk* chunk = nullptr;
    FRawIndexBuffer* sectionIndexBuffer = nullptr;
    std::vector<FSkinVertexMeta>* chunkVertices = nullptr;
    for (int32 sectionIndex = 0; sectionIndex < lod.Sections.size(); sectionIndex++)
    {
      FSkelMeshSection& existingSection = lod.Sections[sectionIndex];
      FSkelMeshChunk& existingChunk = lod.Chunks[existingSection.ChunkIndex];
      if (existingSection.MaterialIndex == face.materialIndex)
      {
        std::vector<uint16> uniqueBones;
        for (int32 vertexIndex = 0; vertexIndex < 3; vertexIndex++)
        {
          for (int32 influenceIndex = wedgeInfluenceIndices[face.wedgeIndices[vertexIndex]]; influenceIndex < importData->Influences.size(); influenceIndex++)
          {
            const auto& influence = importData->Influences[influenceIndex];
            if (influence.vertexIndex != importData->Wedges[face.wedgeIndices[vertexIndex]].pointIndex)
            {
              break;
            }
            if (std::find(existingChunk.BoneMap.begin(), existingChunk.BoneMap.end(), importData->Fbx2GpkBoneMap[influence.boneIndex]) == existingChunk.BoneMap.end())
            {
              uniqueBones.emplace_back(importData->Fbx2GpkBoneMap[influence.boneIndex]);
            }
          }
        }

        if (existingChunk.BoneMap.size() + uniqueBones.size() <= MAX_GPUSKIN_BONES)
        {
          section = &existingSection;
          chunk = &existingChunk;
          sectionIndexBuffer = sectionIndexBufferArray[sectionIndex];
          chunkVertices = chunkVerticesArray[existingSection.ChunkIndex];
          break;
        }
      }
    }

    if (!section)
    {
      section = &lod.Sections.emplace_back();
      section->MaterialIndex = face.materialIndex;

      sectionIndexBuffer = new FRawIndexBuffer();
      sectionIndexBufferArray.push_back(sectionIndexBuffer);

      chunk = &lod.Chunks.emplace_back();
      section->ChunkIndex = lod.Chunks.size() - 1;
      chunkVertices = new std::vector<FSkinVertexMeta>();
      chunkVerticesArray.push_back(chunkVertices);
    }

    uint32 triangleIndices[3] = { 0 };

    for (int32 vertexIndex = 0; vertexIndex < 3; vertexIndex++)
    {
      FSoftSkinVertex	vertex;
      FVector tangentX, tangentY, tangentZ;
      vertex.Position = importData->Points[importData->Wedges[face.wedgeIndices[vertexIndex]].pointIndex];
      if (importData->ImportTangents)
      {
        tangentX = face.tangentX[vertexIndex];
        tangentY = face.tangentY[vertexIndex];
        tangentZ = face.tangentZ[vertexIndex];

        tangentX.Normalize();
        tangentY.Normalize();
        tangentZ.Normalize();
      }
      else
      {
        tangentX = vertexTangentX[vertexIndex].SafeNormal();
        tangentY = vertexTangentY[vertexIndex].SafeNormal();
        tangentZ = vertexTangentZ[vertexIndex].SafeNormal();

        tangentY -= tangentX * (tangentX | tangentY);
        tangentY.Normalize();

        tangentX -= tangentZ * (tangentZ | tangentX);
        tangentY -= tangentZ * (tangentZ | tangentY);

        tangentX.Normalize();
        tangentY.Normalize();
      }

      vertex.TangentX = tangentX;
      vertex.TangentY = tangentY;
      vertex.TangentZ = tangentZ;

      std::memcpy(vertex.UVs, importData->Wedges[face.wedgeIndices[vertexIndex]].UV, sizeof(FVector2D) * MAX_TEXCOORDS);
      //vertex.Color = importData->Wedges[face.wedgeIndices[vertexIndex]].Color;

      {
        int32 infIdx = wedgeInfluenceIndices[face.wedgeIndices[vertexIndex]];
        int32 lookIdx = infIdx;

        uint32 influenceCount = 0;
        while (importData->Influences.size() > lookIdx && lookIdx >= 0 &&
          (importData->Influences[lookIdx].vertexIndex == importData->Wedges[face.wedgeIndices[vertexIndex]].pointIndex))
        {
          influenceCount++;
          lookIdx++;
        }
        influenceCount = std::min<uint32>(influenceCount, MAX_INFLUENCES);

        vertex.InfluenceBones[0] = 0;
        vertex.InfluenceWeights[0] = 255;
        for (uint32 i = 1; i < MAX_INFLUENCES; i++)
        {
          vertex.InfluenceBones[i] = 0;
          vertex.InfluenceWeights[i] = 0;
        }

        uint32 totalInfluenceWeight = 0;
        for (uint32 i = 0; i < influenceCount; i++)
        {
          uint8 boneIndex = (uint8)importData->Fbx2GpkBoneMap[importData->Influences[infIdx + i].boneIndex];
          if (boneIndex >= RefSkeleton.size())
            continue;

          if (std::find(lod.ActiveBoneIndices.begin(), lod.ActiveBoneIndices.end(), boneIndex) == lod.ActiveBoneIndices.end())
          {
            lod.ActiveBoneIndices.emplace_back(boneIndex);
          }
          auto it = std::find(chunk->BoneMap.begin(), chunk->BoneMap.end(), boneIndex);
          if (it == chunk->BoneMap.end())
          {
            chunk->BoneMap.emplace_back(boneIndex);
            vertex.InfluenceBones[i] = chunk->BoneMap.size() - 1;
          }
          else
          {
            vertex.InfluenceBones[i] = it - chunk->BoneMap.begin();
          }
          vertex.InfluenceWeights[i] = (uint8)(importData->Influences[infIdx + i].weight * 255.0f);
          totalInfluenceWeight += vertex.InfluenceWeights[i];
        }
        vertex.InfluenceWeights[0] += 255 - totalInfluenceWeight;
      }
      FSkinVertexMeta vertexMeta = { vertex, importData->Wedges[vertexIndex].pointIndex };
      int32	skinVertexIndex = AddSkinVertex(*chunkVertices, vertexMeta);
      if (skinVertexIndex > (uint32)UINT32_MAX)
      {
        lod.IndexContainer.SetElementSize(sizeof(uint32));
      }
      triangleIndices[vertexIndex] = (uint32)skinVertexIndex;
    }

    if (triangleIndices[0] != triangleIndices[1] && triangleIndices[0] != triangleIndices[2] && triangleIndices[1] != triangleIndices[2])
    {
      for (uint32 vertexIndex = 0; vertexIndex < 3; vertexIndex++)
      {
        sectionIndexBuffer->AddIndex(triangleIndices[vertexIndex]);
      }
    }
  }

  std::qsort(lod.Sections.data(), lod.Sections.size(), sizeof(decltype(lod.Sections)::value_type), [](const void* a, const void* b)
  {
    const FSkelMeshSection* A = (const FSkelMeshSection*)a;
    const FSkelMeshSection* B = (const FSkelMeshSection*)b;
    return A->MaterialIndex != B->MaterialIndex ? A->MaterialIndex - B->MaterialIndex : A->ChunkIndex - B->ChunkIndex;
  });

  std::vector<FRawIndexBuffer*> tmpSectionIndexBufferArray = sectionIndexBufferArray;
  sectionIndexBufferArray.clear();
  for (int32 sectionIndex = 0; sectionIndex < lod.Sections.size(); sectionIndex++)
  {
    FSkelMeshSection& existingSection = lod.Sections[sectionIndex];
    sectionIndexBufferArray.push_back(tmpSectionIndexBufferArray[existingSection.ChunkIndex]);
  }
  std::vector<std::vector<FSkinVertexMeta>*> tmpChunkVerticesArray = chunkVerticesArray;
  chunkVerticesArray.clear();
  for (int32 sectionIndex = 0; sectionIndex < lod.Sections.size(); sectionIndex++)
  {
    FSkelMeshSection& existingSection = lod.Sections[sectionIndex];
    chunkVerticesArray.push_back(tmpChunkVerticesArray[existingSection.ChunkIndex]);
  }
  std::vector<FSkelMeshChunk> tmpChunks = lod.Chunks;
  lod.Chunks.clear();
  for (int32 sectionIndex = 0; sectionIndex < lod.Sections.size(); sectionIndex++)
  {
    FSkelMeshSection& existingSection = lod.Sections[sectionIndex];
    lod.Chunks.emplace_back(tmpChunks[existingSection.ChunkIndex]);
    existingSection.ChunkIndex = lod.Chunks.size() - 1;
  }

  std::vector<std::vector<uint32>> vertexIndexRemap;
  std::vector<uint32> rawPointIndices;
  lod.NumVertices = 0;

  int32 prevMaterialIndex = -1;
  int32 currentChunkBaseVertexIndex = -1;
  int32 currentChunkVertexCount = -1;
  int32 currentVertexIndex = 0;

  for (int32 sectionIndex = 0; sectionIndex < lod.Sections.size(); sectionIndex++)
  {
    FSkelMeshSection& section = lod.Sections[sectionIndex];
    int32 chunkIndex = section.ChunkIndex;
    FSkelMeshChunk& chunk = lod.Chunks[chunkIndex];
    std::vector<FSkinVertexMeta>& chunkVertices = *chunkVerticesArray[chunkIndex];

    currentVertexIndex = 0;
    currentChunkVertexCount = 0;
    prevMaterialIndex = section.MaterialIndex;

    chunk.BaseVertexIndex = currentChunkBaseVertexIndex = lod.NumVertices;
    lod.NumVertices += chunkVertices.size();

    std::vector<uint32>& chunkVertexIndexRemap = vertexIndexRemap.emplace_back();
    chunkVertexIndexRemap.resize(chunkVertices.size());
    for (int32 vertexIndex = 0; vertexIndex < chunkVertices.size(); vertexIndex++)
    {
      const FSkinVertexMeta& meta = chunkVertices[vertexIndex];
      const FSoftSkinVertex& vertex = meta.Vertex;
      if (vertex.InfluenceWeights[1] == 0)
      {
        FRigidSkinVertex rigidVertex;
        rigidVertex.Position = vertex.Position;
        rigidVertex.TangentX = vertex.TangentX;
        rigidVertex.TangentY = vertex.TangentY;
        rigidVertex.TangentZ = vertex.TangentZ;
        std::memcpy(rigidVertex.UVs, vertex.UVs, sizeof(FVector2D)* MAX_TEXCOORDS);
        rigidVertex.Color = vertex.Color;
        rigidVertex.Bone = vertex.InfluenceBones[0];
        chunk.RigidVertices.emplace_back(rigidVertex);
        chunkVertexIndexRemap[vertexIndex] = (uint32)(chunk.BaseVertexIndex + currentVertexIndex);
        currentVertexIndex++;
        rawPointIndices.emplace_back(meta.PointWedgeIdx);
      }
    }
    for (int32 vertexIndex = 0; vertexIndex < chunkVertices.size(); vertexIndex++)
    {
      const FSkinVertexMeta& vertexMeta = chunkVertices[vertexIndex];
      const FSoftSkinVertex& softVertex = vertexMeta.Vertex;
      if (softVertex.InfluenceWeights[1] > 0)
      {
        chunk.SoftVertices.emplace_back(softVertex);
        chunkVertexIndexRemap[vertexIndex] = (uint32)(chunk.BaseVertexIndex + currentVertexIndex);
        currentVertexIndex++;
        rawPointIndices.emplace_back(vertexMeta.PointWedgeIdx);
      }
    }

    chunk.NumRigidVertices = chunk.RigidVertices.size();
    chunk.NumSoftVertices = chunk.SoftVertices.size();

    chunk.CalcMaxBoneInfluences();
  }

  for (int32 chunkIndex = 0; chunkIndex < lod.Chunks.size(); chunkIndex++)
  {
    delete chunkVerticesArray[chunkIndex];
    chunkVerticesArray[chunkIndex] = nullptr;
  }

  uint32 totalIndices = 0;
  for (int32 sectionIndex = 0; sectionIndex < lod.Sections.size(); sectionIndex++)
  {
    FSkelMeshSection& section = lod.Sections[sectionIndex];
    FRawIndexBuffer& sectionIndexBuffer = *sectionIndexBufferArray[sectionIndex];
    totalIndices += sectionIndexBuffer.GetElementCount();
  }

  for (int32 sectionIndex = 0; sectionIndex < lod.Sections.size(); sectionIndex++)
  {
    FSkelMeshSection& section = lod.Sections[sectionIndex];
    FRawIndexBuffer& sectionIndexBuffer = *sectionIndexBufferArray[sectionIndex];

    if (importData->OptimizeIndexBuffer)
    {
      sectionIndexBuffer.SortIndices();
    }

    section.NumTriangles = sectionIndexBuffer.GetElementCount() / 3;
    section.BaseIndex = lod.IndexContainer.GetElementCount();

    for (int32 idx = 0; idx < sectionIndexBuffer.GetElementCount(); ++idx)
    {
      uint32 vertexIndex = vertexIndexRemap[section.ChunkIndex][sectionIndexBuffer.GetIndex(idx)];
      DBreakIf(vertexIndex >= lod.NumVertices);
      lod.IndexContainer.AddIndex(vertexIndex);
    }

    if (section.NumTriangles == 0)
    {
      error = FString::Sprintf("Failed to build section #%d!", sectionIndex);
      return false;
    }
  }

  for (int32 sectionIndex = 0; sectionIndex < lod.Sections.size(); sectionIndex++)
  {
    delete sectionIndexBufferArray[sectionIndex];
  }

  lod.VertexBufferGPUSkin.ElementCount = lod.NumVertices;
  lod.VertexBufferGPUSkin.AllocateBuffer();

  const auto tmpVerts = lod.GetVertices(false);
  for (int32 idx = 0; idx < tmpVerts.size(); ++idx)
  {
    lod.VertexBufferGPUSkin.SetVertex(idx, tmpVerts[idx], importData->BinormalsByUV, importData->FlipBinormals);
  }

  int32 requiredBoneCount = RefSkeleton.size();
  lod.RequiredBones.resize(requiredBoneCount);
  for (int32 idx = 0; idx < requiredBoneCount; idx++)
  {
    lod.RequiredBones[idx] = idx;
  }

  std::swap(LodModels[lodIdx], lod);

  for (FSkelMeshChunk& chunk : LodModels[lodIdx].Chunks)
  {
    for (FRigidSkinVertex& v : chunk.RigidVertices)
    {
      v.BoneMap = &chunk.BoneMap;
    }
    for (FSoftSkinVertex& v : chunk.SoftVertices)
    {
      v.BoneMap = &chunk.BoneMap;
    }
  }

  Materials = importData->ObjectMaterials;
#if _DEBUG
  LodModels[lodIdx]._DebugVerify(this);
#endif

  MarkDirty();
  return true;
}

bool USkeletalMesh::ValidateVisitor(MeshTravallerData* importData, uint32 lodIdx, FString& error, bool& askUser, int32 warningIndex)
{
  int32 widx = 0;
  askUser = false;
  error = "";
#define SHOW_WARN_IF(check, msg, ask)\
if (check) {\
  widx++;\
  if (widx > warningIndex) {\
    error = msg;\
    askUser = ask;\
    return true;\
  }\
}//

#define SHOW_ERROR_IF(check, msg)\
if (check) {\
  widx++;\
  if (widx > warningIndex) {\
    error = msg;\
    return false;\
  }\
}//

  importData->Fbx2GpkBoneMap.clear();
  for (int32 fbxBoneIdx = 0; fbxBoneIdx < importData->Bones.size(); ++fbxBoneIdx)
  {
    int32 foundIndex = -1;
    bool foundMisplaced = false;
    const RawBone& fbxBone = importData->Bones[fbxBoneIdx];
    const FString fbxName = fbxBone.boneName.Split(':').back().ToUpper();
    
    for (int32 gpkBoneIdx = 0; gpkBoneIdx < RefSkeleton.size(); ++gpkBoneIdx)
    {
      const FMeshBone& gpkBone = RefSkeleton[gpkBoneIdx];
      if (gpkBone.Name.String().ToUpper() == fbxName)
      {
        if (abs(gpkBone.BonePos.Position.X - fbxBone.position.X) > SCALE_TOLERANCE ||
            abs(gpkBone.BonePos.Position.Y - fbxBone.position.Y) > SCALE_TOLERANCE || 
            abs(gpkBone.BonePos.Position.Z - fbxBone.position.Z) > SCALE_TOLERANCE)
        {
          bool hasWeights = false;
          for (const RawInfluence& inf : importData->Influences)
          {
            if (inf.boneIndex == fbxBoneIdx)
            {
              hasWeights = true;
              break;
            }
          }
          if (hasWeights)
          {
            foundMisplaced = true;
            continue;
          }
        }
        foundIndex = gpkBoneIdx;
        break;
      }
    }
    if (foundIndex < 0)
    {
      bool hasWeights = false;
      for (const RawInfluence& inf : importData->Influences)
      {
        if (inf.boneIndex == fbxBoneIdx)
        {
          hasWeights = true;
          break;
        }
      }
      if (hasWeights)
      {
        SHOW_ERROR_IF(foundMisplaced, FString("Bone ") + fbxBone.boneName + " position does not match!\nMake sure your model uses same skeleton and it was exported with 1.0 scale.");
        SHOW_ERROR_IF(1, FString("Couldn't find ") + fbxBone.boneName + " bone in the gpk model.\nMake sure you use same skeleton as gpk model.");
      }
      continue;
    }
    importData->Fbx2GpkBoneMap[fbxBoneIdx] = foundIndex;
  }
  return true;
}

bool USkeletalMeshComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(SkeletalMesh, USkeletalMesh*);
  return false;
}

void USkeletalMeshComponent::PostLoad()
{
  LoadObject(SkeletalMesh);
  Super::PostLoad();
}

void FSkelMeshChunk::CalcMaxBoneInfluences()
{
  MaxBoneInfluences = 1;
  for (int32 vertIdx = 0; vertIdx < SoftVertices.size(); vertIdx++)
  {
    FSoftSkinVertex& SoftVert = SoftVertices[vertIdx];
    int32 BonesUsed = 0;
    for (int32 infIdx = 0; infIdx < MAX_INFLUENCES; infIdx++)
    {
      if (SoftVert.InfluenceWeights[infIdx] > 0)
      {
        BonesUsed++;
      }
    }
    for (int32 InfluenceIdx = 0; InfluenceIdx < BonesUsed; InfluenceIdx++)
    {
      if (SoftVert.InfluenceWeights[InfluenceIdx] == 0)
      {
        for (int32 ExchangeIdx = InfluenceIdx + 1; ExchangeIdx < MAX_INFLUENCES; ExchangeIdx++)
        {
          if (SoftVert.InfluenceWeights[ExchangeIdx] != 0)
          {
            std::swap(SoftVert.InfluenceWeights[InfluenceIdx], SoftVert.InfluenceWeights[ExchangeIdx]);
            std::swap(SoftVert.InfluenceBones[InfluenceIdx], SoftVert.InfluenceBones[ExchangeIdx]);
            break;
          }
        }
      }
    }

    MaxBoneInfluences = std::max<int32>(MaxBoneInfluences, BonesUsed);
  }
}

void FSkeletalMeshVertexBuffer::AllocateBuffer(uint32 count)
{
  if (Data)
  {
    delete[] Data;
  }
  if (!count)
  {
    count = ElementCount;
  }
  else
  {
    ElementCount = count;
  }
#define ALLOCATE_BUFFER_TEMPLATE( VertexDataType ) \
  switch(NumTexCoords) \
  { \
    case 1: Data = (FGPUSkinVertexBase*)new VertexDataType<1>[ElementCount]; ElementSize = sizeof(VertexDataType<1>); break; \
    case 2: Data = (FGPUSkinVertexBase*)new VertexDataType<2>[ElementCount]; ElementSize = sizeof(VertexDataType<1>); break; \
    case 3: Data = (FGPUSkinVertexBase*)new VertexDataType<3>[ElementCount]; ElementSize = sizeof(VertexDataType<1>); break; \
    case 4: Data = (FGPUSkinVertexBase*)new VertexDataType<4>[ElementCount]; ElementSize = sizeof(VertexDataType<1>); break; \
  }

  if (!bUseFullPrecisionUVs)
  {
    if (bUsePackedPosition && bDisableCompression)
    {
      ALLOCATE_BUFFER_TEMPLATE(FGPUSkinVertexFloatAB);
    }
    else
    {
      ALLOCATE_BUFFER_TEMPLATE(FGPUSkinVertexFloatABB);
    }
  }
  else
  {
    if (bUsePackedPosition && bDisableCompression)
    {
      ALLOCATE_BUFFER_TEMPLATE(FGPUSkinVertexFloatAAB);
    }
    else
    {
      ALLOCATE_BUFFER_TEMPLATE(FGPUSkinVertexFloatAABB);
    }
  }
}

void FSkeletalMeshVertexBuffer::SetVertex(int32 vertexIndex, const FSoftSkinVertex& src, bool UVSpaceBinormalDir, bool flipBinormals)
{
  FGPUSkinVertexBase* basePtr = (FGPUSkinVertexBase*)((uint8*)Data + vertexIndex * ElementSize);
  basePtr->TangentX = src.TangentX;
  basePtr->TangentZ = src.TangentZ;
  basePtr->TangentZ.Vector.W = UVSpaceBinormalDir ? GetBasisDeterminantSignByte(src.UVs[0], flipBinormals) : GetBasisDeterminantSignByte(src.TangentX, src.TangentY, src.TangentZ, flipBinormals);
  std::memcpy(basePtr->BoneIndex, src.InfluenceBones, sizeof(src.InfluenceBones));
  std::memcpy(basePtr->BoneWeight, src.InfluenceWeights, sizeof(src.InfluenceWeights));
  
  if (!bUseFullPrecisionUVs)
  {
    if (bUsePackedPosition && bDisableCompression)
    {
      FGPUSkinVertexFloatAB<MAX_TEXCOORDS>* typedPtr = (FGPUSkinVertexFloatAB<MAX_TEXCOORDS>*)basePtr;
      typedPtr->Position = src.Position;
      for (uint32 UVIndex = 0; UVIndex < NumTexCoords; ++UVIndex)
      {
        typedPtr->UV[UVIndex] = FVector2DHalf(src.UVs[UVIndex]);
      }
    }
    else
    {
      FGPUSkinVertexFloatABB<MAX_TEXCOORDS>* typedPtr = (FGPUSkinVertexFloatABB<MAX_TEXCOORDS>*)basePtr;
      typedPtr->Position = src.Position;
      for (uint32 UVIndex = 0; UVIndex < NumTexCoords; ++UVIndex)
      {
        typedPtr->UV[UVIndex] = FVector2DHalf(src.UVs[UVIndex]);
      }
    }
  }
  else
  {
    if (bUsePackedPosition && bDisableCompression)
    {
      FGPUSkinVertexFloatAAB<MAX_TEXCOORDS>* typedPtr = (FGPUSkinVertexFloatAAB<MAX_TEXCOORDS>*)basePtr;
      typedPtr->Position = src.Position;
      for (uint32 UVIndex = 0; UVIndex < NumTexCoords; ++UVIndex)
      {
        typedPtr->UV[UVIndex] = FVector2D(src.UVs[UVIndex]);
      }
    }
    else
    {
      FGPUSkinVertexFloatAABB<MAX_TEXCOORDS>* typedPtr = (FGPUSkinVertexFloatAABB<MAX_TEXCOORDS>*)basePtr;
      typedPtr->Position = src.Position;
      for (uint32 UVIndex = 0; UVIndex < NumTexCoords; ++UVIndex)
      {
        typedPtr->UV[UVIndex] = FVector2D(src.UVs[UVIndex]);
      }
    }
  }
}

void FSkeletalMeshVertexBuffer::GetVertex(int32 vertexIndex, FSoftSkinVertex& dst) const
{
  const FGPUSkinVertexBase* basePtr = (const FGPUSkinVertexBase*)((uint8*)Data + vertexIndex * ElementSize);
  dst.TangentX = basePtr->TangentX;
  dst.TangentZ = basePtr->TangentZ;
  std::memcpy(dst.InfluenceBones, basePtr->BoneIndex, sizeof(dst.InfluenceBones));
  std::memcpy(dst.InfluenceWeights, basePtr->BoneWeight, sizeof(dst.InfluenceWeights));

  if (!bUseFullPrecisionUVs)
  {
    if (bUsePackedPosition && bDisableCompression)
    {
      dst.Position = ((const FGPUSkinVertexFloatAB<MAX_TEXCOORDS>*)(basePtr))->Position;
      for (uint32 UVIndex = 0; UVIndex < NumTexCoords; ++UVIndex)
      {
        dst.UVs[UVIndex] = ((const FGPUSkinVertexFloatAB<MAX_TEXCOORDS>*)(basePtr))->UV[UVIndex];
      }
    }
    else
    {
      dst.Position = ((const FGPUSkinVertexFloatABB<MAX_TEXCOORDS>*)(basePtr))->Position;
      for (uint32 UVIndex = 0; UVIndex < NumTexCoords; ++UVIndex)
      {
        dst.UVs[UVIndex] = ((const FGPUSkinVertexFloatABB<MAX_TEXCOORDS>*)(basePtr))->UV[UVIndex];
      }
    }
  }
  else
  {
    if (bUsePackedPosition && bDisableCompression)
    {
      dst.Position = ((FGPUSkinVertexFloatAAB<MAX_TEXCOORDS>*)(basePtr))->Position;
      for (uint32 UVIndex = 0; UVIndex < NumTexCoords; ++UVIndex)
      {
        dst.UVs[UVIndex] = ((FGPUSkinVertexFloatAAB<MAX_TEXCOORDS>*)(basePtr))->UV[UVIndex];
      }
    }
    else
    {
      dst.Position = ((FGPUSkinVertexFloatAABB<MAX_TEXCOORDS>*)(basePtr))->Position;
      for (uint32 UVIndex = 0; UVIndex < NumTexCoords; ++UVIndex)
      {
        dst.UVs[UVIndex] = ((FGPUSkinVertexFloatAABB<MAX_TEXCOORDS>*)(basePtr))->UV[UVIndex];
      }
    }
  }
}
