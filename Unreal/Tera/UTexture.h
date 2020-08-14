#pragma once
#include "UObject.h"

class UTexture : public UObject {
public:
  DECL_UOBJ(UTexture, UObject);

  UPROP(bool, SRGB, true);

  void RegisterProperty(FPropertyTag* property) override;

protected:
  void Serialize(FStream& s) override;
  void PostLoad() override;

protected:
  
  FByteBulkData SourceArt;

};

struct FTexture2DMipMap
{
  FByteBulkData Data;
  int32 DataSize = 0;
	int32 SizeX = 0;
	int32 SizeY = 0;

	void Serialize(FStream& s, UObject* owner, int32 mipIdx);
};

class UTexture2D : public UTexture {
public:
  DECL_UOBJ(UTexture2D, UTexture);

  ~UTexture2D() override;

  void RegisterProperty(FPropertyTag* property) override;

protected:
  void Serialize(FStream& s) override;
  void PostLoad() override;

protected:
  
  UPROP(EPixelFormat, Format, PF_Unknown);
  UPROP(int32, SizeX, 0);
  UPROP(int32, SizeY, 0);

  FString SourceFilePath;
  std::vector<FTexture2DMipMap*> Mips;
  std::vector<FTexture2DMipMap*> CachedMips;
  std::vector<FTexture2DMipMap*> CachedAtiMips;
  FByteBulkData CachedFlashMip;
  std::vector<FTexture2DMipMap*> CachedEtcMips;
  FGuid TextureFileCacheGuid;
  int32 FirstResourceMemMip = 0;
  int32 MaxCachedResolution = 0;
};