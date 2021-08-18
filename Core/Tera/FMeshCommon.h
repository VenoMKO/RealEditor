#pragma once
#include "Core.h"
#include "FStructs.h"

#include <Utils/MeshTravaller.h>

#include <functional>

#define SCALE_TOLERANCE .1f
#define THRESH_POINTS_ARE_SAME (0.002f)
#define THRESH_NORMALS_ARE_SAME (0.00002f)


bool PointsEqual(const FVector& a, const FVector& b, bool bNoEpsilon = false);

bool NormalsEqual(const FVector& a, const FVector& b);

bool UVsEqual(const FVector2D& a, const FVector2D& b);

bool SkeletalMesh_UVsEqual(const struct RawWedge& V1, const struct RawWedge& V2, const int32 UVIndex = 0);

struct VJointPos
{
  FQuat Orientation;
  FVector Position;

  float Length = 0;
  float XSize = 0;
  float YSize = 0;
  float ZSize = 0;

  friend class FStream& operator<<(class FStream& s, VJointPos& p);
};

struct VJointPosExport
{
  FQuat Orientation;
  FVector Position;

  float Length = 0;
  float XSize = 0;
  float YSize = 0;
  float ZSize = 0;

  VJointPosExport() = default;

  VJointPosExport(const VJointPos& pos)
    : Orientation(pos.Orientation)
    , Position(pos.Position)
    , Length(pos.Length)
    , XSize(pos.XSize)
    , YSize(pos.YSize)
    , ZSize(pos.ZSize)
  {}

  friend class FStream& operator<<(class FStream& s, VJointPosExport& p);
};

struct VTriangle {
  uint32 WedgeIndex[3] = { 0 };
  uint8 MatIndex = 0;
  FVector TangentX[3];
  FVector TangentY[3];
  FVector TangentZ[3];
};

struct VTriangleExport {
  uint16 WedgeIndex[3] = { 0 };
  uint8 MatIndex = 0;
  uint8 AuxMatIndex = 0;
  uint32 SmoothingGroups = 1;

  VTriangleExport() = default;
  VTriangleExport(const VTriangle& t)
  {
    WedgeIndex[0] = (uint16)t.WedgeIndex[0];
    WedgeIndex[1] = (uint16)t.WedgeIndex[1];
    WedgeIndex[2] = (uint16)t.WedgeIndex[2];
    MatIndex = t.MatIndex;
  }

  friend class FStream& operator<<(class FStream& s, VTriangleExport& t);
};

struct VVertex {
  int32 VertexIndex = 0;
  FVector2D UVs[MAX_TEXCOORDS];
  FColor Color;
  uint8 MatIndex = 0;
  uint8 Reserved = 0;
  FVector TangentZ;

  bool operator<(const VVertex& b) const
  {
    return VertexIndex < b.VertexIndex;
  }

  friend class FStream& operator<<(class FStream& s, VVertex& v);
};

struct VVertexExport {
  uint32 PointIndex = 0;
  FVector2D UVs;
  uint8 MatIndex = 0;
  uint8 Reserved = 0;
  uint16 Padding = 0;

  VVertexExport() = default;

  VVertexExport(const VVertex& v)
  {
    PointIndex = (uint32)v.VertexIndex;
    UVs = v.UVs[0];
    MatIndex = v.MatIndex;
    Reserved = v.Reserved;
  }

  bool operator<(const VVertexExport& b) const
  {
    return PointIndex < b.PointIndex;
  }

  friend class FStream& operator<<(class FStream& s, VVertexExport& v);
};

struct FVertInfluence {
  float Weight = 0.;
  uint32 VertIndex = 0;
  uint16 BoneIndex = 0;
  uint16 Padding = 0;

  FVertInfluence() = default;
  FVertInfluence(float weight, uint32 vertIndex, uint16 boneIndex)
    : Weight(weight)
    , VertIndex(vertIndex)
    , BoneIndex(boneIndex)
  {}

  friend class FStream& operator<<(class FStream& s, FVertInfluence& b);
};

struct FNamedBoneBinary {
  char Name[64] = { 0 };
  uint32 Flags = 0;
  int32 NumChildren = 0;
  int32 ParentIndex = 0;
  VJointPosExport BonePos;

  void SetName(const char* name)
  {
    memset(Name, 0, 64);
    memcpy(Name, name, std::min<size_t>(strlen(name), 64));
    Name[63] = 0;
  }

  friend class FStream& operator<<(class FStream& s, FNamedBoneBinary& b);
};

struct VBone {
  FString Name;
  VJointPos Transform;
  int32 ParentIndex = 0;
  int32 NumChildren = 0;
  int32 Depth = 0;
};

struct VMaterial {

  void SetName(const char* name)
  {
    memset(MaterialName, 0, 64);
    memcpy(MaterialName, name, std::min<size_t>(strlen(name), 64));
    MaterialName[63] = 0;
  }

  char MaterialName[64] = { 0 };
  int32 TextureIndex = 0;
  uint32 PolyFlags = 0;
  int32 AuxMaterial = 0;
  uint32 AuxFlags = 0;
  int32 LodBias = 0;
  int32 LodStyle = 0;

  friend class FStream& operator<<(class FStream& s, VMaterial& v);
};

struct MeshExportContext {
  std::wstring Path;
  bool EmbedMedia = false;
  bool ExportSkeleton = true;
  bool ExportLods = false;
  bool ExportCollisions = true;
  bool ExportLightMapUVs = false;

  // Animation related settings
  class USkeletalMesh* Skeleton = nullptr;
  bool ExportMesh = true;
  bool SplitTakes = true;
  bool CompressTracks = true;
  bool ResampleTracks = false;
  float TrackRateScale = 1.;
  bool InverseAnimQuatW = false;

  bool ApplyRootTransform = false;
  FVector PrePivot;
  FVector Translation;
  FVector Scale3D = FVector(1, 1, 1);
  FRotator Rotation;

  std::function<void(const FString)> ProgressDescFunc;
  std::function<void(int32)> ProgressMaxFunc;
  std::function<void(int32)> ProgressFunc;

  std::string Error;
};

struct MeshImportContext {
  std::wstring Path;
  std::string Error;
  MeshTravallerData ImportData;
  FVector Scale3D = FVector(1, 1, 1);
};

struct VQuatAnimKey {
  FVector Position;
  FQuat Orientation;
  float Time = 1.f;

  friend class FStream& operator<<(class FStream& s, VQuatAnimKey& k);
};

struct AnimInfoBinary {
  char Name[64] = { 0 };
  char Group[64] = { 0 };
  int32 TotalBones = 0;
  int32 ScaleInclude = 0;
  int32 KeyCompressionStyle = 0;
  int32 KeyQuotum = 0;
  float KeyReduction = 0.f;
  float TrackTime = 0.f;
  float AnimRate = 0.f;
  int32 StartBone = 0;
  int32 FirstRawFrame = 0;
  int32 NumRawFrames = 0;

  void SetName(const char* name)
  {
    memset(Name, 0, 64);
    memcpy(Name, name, std::min<size_t>(strlen(name), 64));
    Name[63] = 0;
  }

  void SetGroup(const char* name)
  {
    memset(Group, 0, 64);
    memcpy(Group, name, std::min<size_t>(strlen(name), 64));
    Group[63] = 0;
  }

  friend class FStream& operator<<(class FStream& s, AnimInfoBinary& i);
};