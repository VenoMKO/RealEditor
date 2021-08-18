#pragma once
#include "MeshUtils.h"

class PskUtils : public MeshUtils {
public:

  bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, MeshExportContext& ctx) override;
  bool ExportStaticMesh(UStaticMesh* sourceMesh, MeshExportContext& ctx) override;

  bool ExportAnimationSequence(USkeletalMesh* sourceMesh, UAnimSequence* sequence, MeshExportContext& ctx) override;
  bool ExportAnimationSet(USkeletalMesh* sourceMesh, UAnimSet* animSet, MeshExportContext& ctx) override;

protected:
  bool ExportSequence(USkeletalMesh* sourceMesh, UAnimSequence* sequence, MeshExportContext& ctx, class FStream& s);
};