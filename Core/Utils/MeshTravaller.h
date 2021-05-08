#pragma once
#include <Tera/Core.h>
#include <Tera/FStructs.h>

struct RawTriangle {
  int32 wedgeIndices[3] = { 0 };
  int16 materialIndex = 0;

  FVector tangentX[3] = { 0 };
  FVector tangentY[3] = { 0 };
  FVector tangentZ[3] = { 0 };
  float basis[3] = { 0 };
};

struct RawInfluence {
  float weight = 0.;
  int32 vertexIndex = 0;
  int32 boneIndex = 0;
};


struct RawWedge {
  int32 pointIndex = 0;
  int32 materialIndex = 0;
  FVector2D UV[MAX_TEXCOORDS] = { 0 };
};

struct RawBone {
  FString boneName;
  int32 boneIndex = 0;
  int32 parentIndex = 0;
  FVector position;
  FQuat orientation;
};

struct RawMaterial {
  FString materialName;
  int32 materialIndex = 0;
};

struct MeshTravallerData {
  bool ImportSkeleton = false;

  std::vector<FVector> Points;
  std::vector<RawTriangle> Faces;
  std::vector<int32> Indices;
  std::vector<FString> Materials;
  std::vector<RawBone> Bones;
  std::vector<RawWedge> Wedges;
  std::vector<RawInfluence> Influences;

  int32 UVSetCount = 0;

  bool FlipTangents = false;
  bool MissingUVs = false;
  bool MissingNormals = false;
  bool MissingTangents = false;

  std::vector<std::pair<FString, UObject*>> MaterialMap;
};