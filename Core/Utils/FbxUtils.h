#pragma once
#include "MeshUtils.h"

class FbxUtils : public MeshUtils {
public:
  FbxUtils();
  ~FbxUtils();

  bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, MeshExportContext& ctx) override;
  bool ExportStaticMesh(UStaticMesh* sourceMesh, MeshExportContext& ctx) override;
  
  bool ExportAnimationSequence(USkeletalMesh* sourceMesh, UAnimSequence* sequence, MeshExportContext& ctx) override;
  bool ExportAnimationSet(USkeletalMesh* sourceMesh, UAnimSet* animSet, MeshExportContext& ctx) override;

  bool ImportSkeletalMesh(MeshImportContext& ctx) override;
  bool ImportStaticMesh(MeshImportContext& ctx) override;

protected:
  bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, MeshExportContext& ctx, void** outNode);
  bool ExportStaticMesh(UStaticMesh* sourceMesh, int32 lodIdx, MeshExportContext& ctx, void** outNode);
  bool ExportCollision(UStaticMesh* sourceMesh, MeshExportContext& ctx, const char* meshName, std::vector<void*>& outNodes);
  bool ExportSequence(USkeletalMesh* sourceMesh, UAnimSequence* sequence, void* boneNodes, void* animLayer, MeshExportContext& ctx);
  void ApplyRootTransform(void* node, MeshExportContext& ctx);
  bool SaveScene(const std::wstring& path);

private:
  void* SdkManager = nullptr;
  void* Scene = nullptr;
};