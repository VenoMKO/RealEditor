#include "TextureTravaller.h"
#include <Tera/UTexture.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>

void TextureTravaller::SetFormat(EPixelFormat format)
{
  Format = format;
}

void TextureTravaller::SetAddressX(TextureAddress x)
{
  AddressX = x;
}

void TextureTravaller::SetAddressY(TextureAddress y)
{
  AddressY = y;
}

void TextureTravaller::SetCompression(TextureCompressionSettings compression)
{
  Compression = compression;
}

void TextureTravaller::SetSRGB(bool srgb)
{
  SRGB = srgb;
}

void TextureTravaller::SetRawData(void* data, int32 size, bool transferOwnership)
{
  Data = (uint8*)data;
  Size = size;
  OwnsData = transferOwnership;
}

void TextureTravaller::AddMipMap(int32 sizeX, int32 sizeY, int32 size, void* data)
{
  Mips.push_back({ sizeX, sizeY, size, data });
}

std::string TextureTravaller::GetError() const
{
  return Error;
}

bool TextureTravaller::Visit(UTexture2D* texture)
{
  if (!texture)
  {
    Error = "Texture is NULL.";
    return false;
  }

  // TODO: Create properties if they don't exist and remove nullptr check
  // TODO: Apply AddressMode & CompressionMode changes.

  if (!Mips.size())
  {
    Error = "Texture has no mipmaps.";
    return false;
  }

  // TODO: patch Resource allocation
  texture->GetPackage()->GetTextureAllocations().TextureTypes.clear();
  // Invalidate texture cache
  texture->TextureFileCacheGuid = FGuid();

  if (Format != texture->Format)
  {
    texture->Format = Format;
    if (texture->FormatProperty)
    {
      texture->FormatProperty->Value->GetByte() = Format;
    }
  }

  if (Mips[0].SizeX != texture->SizeX)
  {
    texture->SizeX = Mips[0].SizeX;
    if (texture->SizeXProperty)
    {
      texture->SizeXProperty->Value->GetInt() = Mips[0].SizeX;
    }
  }

  if (Mips[0].SizeY != texture->SizeY)
  {
    texture->SizeY = Mips[0].SizeY;
    if (texture->SizeYProperty)
    {
      texture->SizeYProperty->Value->GetInt() = Mips[0].SizeY;
    }
  }

  if (SRGB != texture->SRGB)
  {
    texture->SRGB = SRGB;
    if (texture->SRGBProperty)
    {
      texture->SRGBProperty->Value->GetBool() = SRGB;
    }
  }

  if (Mips.size() - 1 != texture->MipTailBaseIdx)
  {
    texture->MipTailBaseIdx = Mips.size() - 1;
    if (texture->MipTailBaseIdxProperty)
    {
      texture->MipTailBaseIdxProperty->Value->GetInt() = Mips.size() - 1;
    }
  }

  if (texture->TextureFileCacheNameProperty)
  {
    texture->RemoveProperty(texture->TextureFileCacheNameProperty);
    texture->TextureFileCacheName = nullptr;
    texture->TextureFileCacheNameProperty = nullptr;
  }

  if (texture->FirstResourceMemMip != 0)
  {
    texture->FirstResourceMemMip = 0;
    if (texture->FirstResourceMemMipProperty)
    {
      texture->FirstResourceMemMipProperty->Value->GetInt() = 0;
    }
  }
  // We pulled all the data, so it should be safe to turn off fexp flag
  texture->Export->ExportFlags &= ~EF_ForcedExport;

  texture->DeleteStorage();
  texture->TextureResource = nullptr;

  for (TMipMap& tmip : Mips)
  {
    FTexture2DMipMap* mip = new FTexture2DMipMap();
    mip->SizeX = tmip.SizeX;
    mip->SizeY = tmip.SizeY;

    void* data = malloc(tmip.Size);
    memcpy(data, tmip.Data, tmip.Size);
    mip->Data = new FByteBulkData(texture->GetPackage(), BULKDATA_SerializeCompressedLZO, tmip.Size, data, true);
    texture->Mips.push_back(mip);
  }
  texture->MarkDirty();
  return true;
}


