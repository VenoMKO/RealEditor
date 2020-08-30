#pragma once
#include <Tera/Core.h>
#include <Tera/FStructs.h>

class UTexture2D;
class TextureTravaller {
public:
  void SetFormat(EPixelFormat format);
  void SetAddressX(TextureAddress x);
  void SetAddressY(TextureAddress y);
  void SetCompression(TextureCompressionSettings compression);
  void SetSRGB(bool srgb);

  void SetRawData(void* data, int32 size, bool transferOwnership = false);
  void AddMipMap(int32 sizeX, int32 sizeY, int32 size, void* data);

  std::string GetError() const;

  bool Visit(UTexture2D* texture);

private:
  struct TMipMap {
    int32 SizeX = 0;
    int32 SizeY = 0;
    int32 Size = 0;
    void* Data = nullptr;
  };

private:
  std::string Error;

  EPixelFormat Format = PF_Unknown;
  TextureAddress AddressX = TA_Wrap;
  TextureAddress AddressY = TA_Wrap;
  TextureCompressionSettings Compression = TC_Default;
  bool SRGB = false;

  bool OwnsData = false;
  uint8* Data = nullptr;
  int32 Size = 0;
  std::vector<TMipMap> Mips;
};