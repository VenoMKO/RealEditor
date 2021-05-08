#pragma once
#include <Tera/USkeletalMesh.h>
#include <Tera/UStaticMesh.h>

#include <Utils/MeshTravaller.h>

struct FbxImportContext {
  std::wstring Path;
  std::string Error;
  bool ImportTangents = true;
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

  bool ApplyRootTransform = false;
  FVector PrePivot;
  FVector Translation;
  FVector Scale3D = FVector(1, 1, 1);
  FRotator Rotation;

  std::string Error;
};

class FbxUtils {
public:
  FbxUtils();
  ~FbxUtils();

  bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, FbxExportContext& ctx);
  bool ExportStaticMesh(UStaticMesh* sourceMesh, FbxExportContext& ctx);

  bool ImportSkeletalMesh(FbxImportContext& ctx);
  bool ImportStaticMesh(FbxImportContext& ctx);

  bool SaveScene(const std::wstring& path, bool embedMedia);

protected:
  bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, FbxExportContext& ctx, void** outNode);
  bool ExportStaticMesh(UStaticMesh* sourceMesh, int32 lodIdx, FbxExportContext& ctx, void** outNode);
  bool ExportCollision(UStaticMesh* sourceMesh, FbxExportContext& ctx, const char* meshName, std::vector<void*>& outNodes);
  void ApplyRootTransform(void* node, FbxExportContext& ctx);

private:
  void* SdkManager = nullptr;
  void* Scene = nullptr;
};