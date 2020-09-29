#pragma once
#include <Tera/USkeletalMesh.h>

struct FbxExportContext {
  std::wstring Path;
  bool EmbedMedia = false;
  bool ExportSkeleton = false;
  uint32 LodIndex = 0;

  std::string Error;
};

class FbxUtils {
public:
  FbxUtils();
  ~FbxUtils();

  bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, FbxExportContext& ctx);

  bool SaveScene(const std::wstring& path, bool embedMedia);

protected:
  bool ExportSkeletalMesh(USkeletalMesh* sourceMesh, FbxExportContext& ctx, void** outNode);

private:
  void* SdkManager = nullptr;
  void* Scene = nullptr;
};