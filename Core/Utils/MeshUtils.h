#pragma once
#include <Tera/USkeletalMesh.h>
#include <Tera/UStaticMesh.h>
#include <Tera/UAnimation.h>

#include "MeshTravaller.h"

#include <functional>

enum MeshExporterType {
  MET_Fbx = 0,
  MET_Psk = 1
};

class MeshUtils {
public:
  static MeshExporterType GetExporterTypeFromExtension(const std::string& ext);
  static std::unique_ptr<MeshUtils> CreateUtils(MeshExporterType type);

  virtual ~MeshUtils() = default;

  virtual bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, MeshExportContext& ctx)
  {
    return Stub(ctx.Error);
  }

  virtual bool ExportStaticMesh(UStaticMesh* sourceMesh, MeshExportContext& ctx)
  {
    return Stub(ctx.Error);
  }

  virtual bool ExportAnimationSequence(USkeletalMesh* sourceMesh, UAnimSequence* sequence, MeshExportContext& ctx)
  {
    return Stub(ctx.Error);
  }

  virtual bool ExportAnimationSet(USkeletalMesh* sourceMesh, UAnimSet* animSet, MeshExportContext& ctx)
  {
    return Stub(ctx.Error);
  }

  virtual bool ImportSkeletalMesh(MeshImportContext& ctx)
  {
    return Stub(ctx.Error);
  }

  virtual bool ImportStaticMesh(MeshImportContext& ctx)
  {
    return Stub(ctx.Error);
  }

protected:
  bool Stub(std::string& err) const
  {
    DBreak();
    err = "Error!\n\nThis format is not supported. Try to use a different format.";
    return false;
  }
};
