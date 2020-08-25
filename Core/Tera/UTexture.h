#pragma once
#include "UObject.h"

#include <GL/glew.h>
#include <osg/Image>

class UTexture : public UObject {
public:
  DECL_UOBJ(UTexture, UObject);

  UPROP(bool, SRGB, true);

  bool RegisterProperty(FPropertyTag* property) override;

protected:
  void Serialize(FStream& s) override;

protected:
  FByteBulkData SourceArt;
};


class UTexture2D : public UTexture {
public:

  static EPixelFormat GetEffectivePixelFormat(const EPixelFormat format, bool bSRGB);

  DECL_UOBJ(UTexture2D, UTexture);

  ~UTexture2D() override;

  bool RegisterProperty(FPropertyTag* property) override;

  UPROP(EPixelFormat, Format, PF_Unknown);
  UPROP(int32, SizeX, 0);
  UPROP(int32, SizeY, 0);
  UPROP(TextureAddress, AddressX, TA_Wrap);
  UPROP(TextureAddress, AddressY, TA_Wrap);
  UPROP(TextureGroup, LODGroup, TEXTUREGROUP_World);
  UPROP(int32, MipTailBaseIdx, -1);
  UPROP(bool, bNoTiling, false);

  osg::ref_ptr<osg::Image> GetTextureResource();

protected:
  void Serialize(FStream& s) override;
  void PostLoad() override;

public:
  std::vector<FTexture2DMipMap*> Mips;
protected:
  FString SourceFilePath;
  std::vector<FTexture2DMipMap*> CachedMips;
  std::vector<FTexture2DMipMap*> CachedAtiMips;
  FByteBulkData CachedFlashMip;
  std::vector<FTexture2DMipMap*> CachedEtcMips;
  FGuid TextureFileCacheGuid;
  int32 FirstResourceMemMip = 0;
  int32 MaxCachedResolution = 0;

  osg::ref_ptr<osg::Image> TextureResource = nullptr;
};
