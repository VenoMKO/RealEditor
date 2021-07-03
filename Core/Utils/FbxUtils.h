#pragma once
#include <Tera/USkeletalMesh.h>
#include <Tera/UStaticMesh.h>
#include <Tera/UAnimation.h>

#include <Utils/MeshTravaller.h>

#include <functional>

struct FbxImportContext {
  std::wstring Path;
  std::string Error;
  MeshTravallerData ImportData;
  FVector Scale3D = FVector(1, 1, 1);
};

struct FbxExportContext {
  std::wstring Path;
  bool EmbedMedia = false;
  bool ExportSkeleton = true;
  bool ExportLods = true;
  bool ExportCollisions = true;
  bool ExportLightMapUVs = false;

  // Animation related settings
  USkeletalMesh* Skeleton = nullptr;
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

class FbxUtils {
public:
  FbxUtils();
  ~FbxUtils();

  bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, FbxExportContext& ctx);
  bool ExportStaticMesh(UStaticMesh* sourceMesh, FbxExportContext& ctx);
  bool ExportAnimationSequence(USkeletalMesh* sourceMesh, UAnimSequence* sequence, FbxExportContext& ctx);
  bool ExportAnimationSet(USkeletalMesh* sourceMesh, UAnimSet* animSet, FbxExportContext& ctx);

  bool ImportSkeletalMesh(FbxImportContext& ctx);
  bool ImportStaticMesh(FbxImportContext& ctx);

  bool SaveScene(const std::wstring& path);

protected:
  bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, FbxExportContext& ctx, void** outNode);
  bool ExportStaticMesh(UStaticMesh* sourceMesh, int32 lodIdx, FbxExportContext& ctx, void** outNode);
  bool ExportCollision(UStaticMesh* sourceMesh, FbxExportContext& ctx, const char* meshName, std::vector<void*>& outNodes);
  bool ExportSequence(USkeletalMesh* sourceMesh, UAnimSequence* sequence, void* boneNodes, void* animLayer, FbxExportContext& ctx);
  void ApplyRootTransform(void* node, FbxExportContext& ctx);

private:
  void* SdkManager = nullptr;
  void* Scene = nullptr;
};