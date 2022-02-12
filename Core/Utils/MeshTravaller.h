#pragma once
#include <Tera/Core.h>
#include <Tera/FStructs.h>

struct RawTriangle {
  int32 wedgeIndices[3] = { 0 };
  int16 materialIndex = 0;

  FVector tangentX[3] = { 0 };
  FVector tangentY[3] = { 0 };
  FVector tangentZ[3] = { 0 };
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
  int32 numChildren = 0;
};

struct RawMaterial {
  FString materialName;
  int32 materialIndex = 0;
};

struct MeshTravallerData {
  // Replace skeleton
  bool ImportSkeleton = false;
  // If false RE will calculate tangents
  bool ImportTangents = true;
  // Invert binormal direction
  bool FlipBinormals = false;
  // Calculate binormal direction based on UV coordinates
  bool BinormalsByUV = true;
  // Average normals of different vertecies sharing the same position. Works only if ImportTangents == true
  bool AverageNormals = false;
  // Optimize index buffer via NvTriStrip
  bool OptimizeIndexBuffer = true;
  // Convert scene coordinate system
  bool ConvertScene = true;
  // Update bounds
  bool CalculateBounds = true;

  std::vector<RawBone> Bones;
  std::vector<FString> Materials;
  std::vector<FVector> Points;
  std::vector<RawTriangle> Faces;
  std::vector<int32> Indices;
  std::vector<RawWedge> Wedges;
  std::vector<RawInfluence> Influences;

  std::map<int32, int32> Fbx2GpkBoneMap;

  int32 UVSetCount = 0;

  bool FlipTangents = false;
  bool MissingUVs = false;
  bool MissingNormals = false;
  bool MissingTangents = false;

  std::vector<std::pair<FString, UObject*>> MaterialMap;
  std::vector<UObject*> ObjectMaterials;
};