#include "USkeletalMesh.h"

void USkeletalMesh::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Bounds;
  s << Materials;
  s << Origin;
  s << RotOrigin;
  s << RefSkeleton;
  s << SkeletalDepth;

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
  s << c.NumSoftVertices;
  s << c.MaxBoneInfluences;
  return s;
}

FStream& operator<<(FStream& s, FMultiSizeIndexContainer& c)
{
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << c.NeedsCPUAccess;
  }
  s << c.ElementSize;
  s << c.ElementCount;
  if (s.IsReading())
  {
    c.AllocateBuffer(c.ElementCount, c.ElementSize);
  }
  s.SerializeBytes(c.Get16BitBuffer(), c.ElementSize * c.ElementCount);
  return s;
}

FStream& operator<<(FStream& s, FMeshEdge& e)
{
  return s << e.Vertices[0] << e.Vertices[1] << e.Faces[0] << e.Faces[1];
}

FStream& operator<<(FStream& s, FSkeletalMeshVertexBuffer& b)
{
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

  return s;
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


}
