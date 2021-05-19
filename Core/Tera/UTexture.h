#pragma once
#include "UObject.h"
#include <Utils/TextureTravaller.h>

#include <array>

namespace osg
{
  class Image;
}

class UTexture : public UObject {
public:
  DECL_UOBJ(UTexture, UObject);

  UPROP_CREATABLE(bool, SRGB, true);
  UPROP_CREATABLE_ENUM(TextureCompressionSettings, CompressionSettings, TC_Default);
  UPROP_CREATABLE_STATIC_ARR(float, 4, UnpackMin, 0,0,0,0);

  bool RegisterProperty(FPropertyTag* property) override;

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

  UPROP_CREATABLE_ENUM(EPixelFormat, Format, PF_Unknown);
  UPROP_CREATABLE(int32, SizeX, 0);
  UPROP_CREATABLE(int32, SizeY, 0);
  UPROP_CREATABLE_ENUM(TextureAddress, AddressX, TA_Wrap);
  UPROP_CREATABLE_ENUM(TextureAddress, AddressY, TA_Wrap);
  UPROP_CREATABLE_ENUM(TextureGroup, LODGroup, TEXTUREGROUP_World);
  UPROP_CREATABLE(int32, MipTailBaseIdx, 0);
  UPROP_CREATABLE(int32, FirstResourceMemMip, 0);
  UPROP_CREATABLE(bool, bNoTiling, false);
  UPROP_CREATABLE(bool, NeverStream, false);
  UPROP(FName*, TextureFileCacheName, nullptr);

  bool RenderTo(osg::Image* target, int32 maxWidth = 0, int32 maxHeight = 0);

  friend bool TextureTravaller::Visit(UTexture2D* texture);

  void Serialize(FStream& s) override;

  unsigned int Hash() const;

  // Disable texture caching, pull max mipmap, delete smaller mips
  // Needed for cross-region mods
  void DisableCaching();

protected:
  void PostLoad() override;
  void DeleteStorage();

public:
  std::vector<FTexture2DMipMap*> Mips;
protected:
  FString SourceFilePath;
  std::vector<FTexture2DMipMap*> CachedMips;
  std::vector<FTexture2DMipMap*> CachedAtiMips;
  FByteBulkData CachedFlashMip;
  std::vector<FTexture2DMipMap*> CachedEtcMips;
  FGuid TextureFileCacheGuid;
  int32 MaxCachedResolution = 0;
};

class UTerrainWeightMapTexture : public UTexture2D {
public:
  DECL_UOBJ(UTerrainWeightMapTexture, UTexture2D);
};

class UTextureCube : public UTexture {
public:
  DECL_UOBJ(UTextureCube, UTexture);
  UPROP(UTexture2D*, FacePosX, nullptr);
  UPROP(UTexture2D*, FaceNegX, nullptr);
  UPROP(UTexture2D*, FacePosY, nullptr);
  UPROP(UTexture2D*, FaceNegY, nullptr);
  UPROP(UTexture2D*, FacePosZ, nullptr);
  UPROP(UTexture2D*, FaceNegZ, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;

  // targets order: FacePosX, FaceNegX, FacePosY, FaceNegY, FacePosZ, FaceNegZ
  bool RenderTo(osg::Image* targets);

  std::array<UTexture2D*, 6> GetFaces()
  {
    return { FacePosX, FaceNegX, FacePosY, FaceNegY, FacePosZ, FaceNegZ };
  }

protected:
  void PostLoad() override;
};