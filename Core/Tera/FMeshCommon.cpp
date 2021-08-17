#include "FMeshCommon.h"
#include "FStream.h"

#include <Utils/MeshTravaller.h>

FStream& operator<<(FStream& s, VJointPos& p)
{
  return s << p.Orientation << p.Position;
}

FStream& operator<<(FStream& s, VJointPosExport& p)
{
  return s << p.Orientation << p.Position << p.Length << p.XSize << p.YSize << p.ZSize;
}

FStream& operator<<(FStream& s, VVertex& v)
{
  s << v.VertexIndex;
  s << v.UVs[0];
  s << v.Color;
  s << v.MatIndex;
  s << v.Reserved;
  return s;
}

FStream& operator<<(FStream& s, VVertexExport& v)
{
  s << v.PointIndex;
  s << v.UVs;
  s << v.MatIndex;
  s << v.Reserved;
  s << v.Padding;
  return s;
}

FStream& operator<<(FStream& s, VTriangleExport& t)
{
  for (uint16& wedge : t.WedgeIndex)
  {
    s << wedge;
  }
  s << t.MatIndex;
  s << t.AuxMatIndex;
  s << t.SmoothingGroups;
  return s;
}

FStream& operator<<(FStream& s, VMaterial& v)
{
  s.SerializeBytes(v.MaterialName, sizeof(v.MaterialName));
  s << v.TextureIndex;
  s << v.PolyFlags;
  s << v.AuxMaterial;
  s << v.AuxFlags;
  s << v.LodBias;
  s << v.LodStyle;
  return s;
}

FStream& operator<<(FStream& s, FNamedBoneBinary& b)
{
  s.SerializeBytes(b.Name, sizeof(b.Name));
  s << b.Flags;
  s << b.NumChildren;
  s << b.ParentIndex;
  s << b.BonePos;
  return s;
}

FStream& operator<<(FStream& s, FVertInfluence& b)
{
  return s << b.Weight << b.VertIndex << b.BoneIndex << b.Padding;
}

bool PointsEqual(const FVector& a, const FVector& b, bool bNoEpsilon)
{
  const float e = bNoEpsilon ? 0.0f : THRESH_POINTS_ARE_SAME * 4.;
  if (fabs(a.X - b.X) > e || fabs(a.Y - b.Y) > e || fabs(a.Z - b.Z) > e)
  {
    return false;
  }

  return true;
}

bool NormalsEqual(const FVector& a, const FVector& b)
{
  const float e = THRESH_NORMALS_ARE_SAME * 4.f;
  if (fabs(a.X - b.X) > e || fabs(a.Y - b.Y) > e || fabs(a.Z - b.Z) > e)
  {
    return false;
  }
  return true;
}

bool UVsEqual(const FVector2D& a, const FVector2D& b)
{
  if (abs(a.X - b.X) > (1.0f / 1024.0f))
  {
    return false;
  }

  if (abs(a.Y - b.Y) > (1.0f / 1024.0f))
  {
    return false;
  }
  return true;
}

bool SkeletalMesh_UVsEqual(const RawWedge& V1, const RawWedge& V2, const int32 UVIndex)
{
  const FVector2D& UV1 = V1.UV[UVIndex];
  const FVector2D& UV2 = V2.UV[UVIndex];

  if (abs(UV1.X - UV2.X) > (1.0f / 1024.0f))
  {
    return false;
  }
  if (abs(UV1.Y - UV2.Y) > (1.0f / 1024.0f))
  {
    return false;
  }
  return true;
}