#pragma once
#include "MeshUtils.h"

class PskUtils : public MeshUtils {
public:

  bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, MeshExportContext& ctx) override;

  bool ExportStaticMesh(UStaticMesh* sourceMesh, MeshExportContext& ctx) override;

};