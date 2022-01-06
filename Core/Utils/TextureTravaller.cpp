#include "TextureTravaller.h"
#include <Tera/UTexture.h>
#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UClass.h>

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

void TextureTravaller::SetIsNew(bool flag)
{
  ConfigureAsNew = flag;
}

void TextureTravaller::SetLODGroup(TextureGroup group)
{
  LODGroup = group;
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
  if (!Mips.size())
  {
    Error = "Texture has no mipmaps.";
    return false;
  }

  if (ConfigureAsNew)
  {
    for (FPropertyTag* p : texture->Properties)
    {
      delete p;
    }
    texture->Properties.clear();
    texture->SourceFilePath = SourcePath;
  }

  // TODO: patch Resource allocation
  texture->GetPackage()->GetTextureAllocations().TextureTypes.clear();
  // Invalidate texture cache
  texture->TextureFileCacheGuid = FGuid();

  if (Compression != TC_Default)
  {
    if (!texture->CompressionSettingsProperty)
    {
      texture->CreatePropertyCompressionSettings(Compression);
    }
    else
    {
      texture->CompressionSettings = Compression;
      texture->CompressionSettingsProperty->GetByte() = (uint8)Compression;
      FString enumName = texture->CompressionSettingsProperty->Value->Enum->GetEnum(Compression).String();
      texture->GetPackage()->GetNameIndex(enumName, true);
    }
    if (Compression == TC_NormalmapAlpha || Compression == TC_Normalmap)
    {
      for (int32 idx = 0; idx < 3; ++idx)
      {
        if (!texture->UnpackMinProperty[idx])
        {
          texture->CreatePropertyUnpackMin(-1., idx);
        }
        else
        {
          texture->UnpackMinProperty[idx]->GetFloat() = -1.;
          texture->UnpackMin[idx] = -1.;
        }
      }
    }
  }

  if (LODGroup != texture->LODGroup)
  {
    if (!texture->LODGroupProperty)
    {
      texture->CreatePropertyLODGroup(LODGroup);
    }
    else
    {
      texture->LODGroup = LODGroup;
      texture->LODGroupProperty->GetByte() = (uint8)LODGroup;
      FString enumName = texture->LODGroupProperty->Value->Enum->GetEnum(LODGroup).String();
      texture->GetPackage()->GetNameIndex(enumName, true);
    }
  }

  if (AddressX != TA_Wrap || AddressY != TA_Wrap)
  {
    if (!texture->AddressXProperty)
    {
      texture->CreatePropertyAddressX(AddressX);
    }
    else
    {
      texture->AddressX = AddressX;
      FString enumName = texture->AddressXProperty->Value->Enum->GetEnum(AddressX).String();
      texture->GetPackage()->GetNameIndex(enumName, true);
    }
    if (!texture->AddressYProperty)
    {
      texture->CreatePropertyAddressY(AddressY);
    }
    else
    {
      texture->AddressY = AddressY;
      FString enumName = texture->AddressYProperty->Value->Enum->GetEnum(Compression).String();
      texture->GetPackage()->GetNameIndex(enumName, true);
    }
  }

  if (!texture->FormatProperty)
  {
    texture->CreatePropertyFormat(Format);
    texture->Format = Format;
  }
  if (Format != texture->Format)
  {
    texture->Format = Format;
    texture->FormatProperty->Value->GetByte() = Format;
    FString enumName = texture->FormatProperty->Value->Enum->GetEnum(Format).String();
    texture->GetPackage()->GetNameIndex(enumName, true);
  }

  if (Mips[0].SizeX != texture->SizeX)
  {
    texture->SizeX = Mips[0].SizeX;
    if (!texture->SizeXProperty)
    {
      texture->CreatePropertySizeX(Mips[0].SizeX);
    }
    else
    {
      texture->SizeXProperty->Value->GetInt() = Mips[0].SizeX;
    }
  }

  if (Mips[0].SizeY != texture->SizeY)
  {
    texture->SizeY = Mips[0].SizeY;
    if (!texture->SizeYProperty)
    {
      texture->CreatePropertySizeY(Mips[0].SizeY);
    }
    else
    {
      texture->SizeYProperty->Value->GetInt() = Mips[0].SizeY;
    }
  }

  if (!SRGB && !texture->SRGBProperty)
  {
    texture->CreatePropertySRGB(SRGB);
  }
  if (SRGB != texture->SRGB)
  {
    texture->SRGB = SRGB;
    texture->SRGBProperty->Value->GetBool() = SRGB;
  }

  if (!texture->MipTailBaseIdxProperty)
  {
    texture->CreatePropertyMipTailBaseIdx(Mips.size() - 1);
  }
  if (Mips.size() - 1 != texture->MipTailBaseIdx)
  {
    texture->MipTailBaseIdx = Mips.size() - 1;
    texture->MipTailBaseIdxProperty->Value->GetInt() = Mips.size() - 1;
  }

  if (texture->TextureFileCacheNameProperty)
  {
    texture->RemoveProperty(texture->TextureFileCacheNameProperty);
    texture->TextureFileCacheName = nullptr;
    texture->TextureFileCacheNameProperty = nullptr;
  }

  if (!texture->FirstResourceMemMipProperty)
  {
    texture->CreatePropertyFirstResourceMemMip(0);
  }
  if (texture->FirstResourceMemMip != 0)
  {
    texture->FirstResourceMemMip = 0;
    texture->FirstResourceMemMipProperty->Value->GetInt() = 0;
  }
  // We pulled all the data, so it should be safe to turn off fexp flag
  texture->Export->ExportFlags &= ~EF_ForcedExport;

  texture->DeleteStorage();

  for (TMipMap& tmip : Mips)
  {
    FTexture2DMipMap* mip = new FTexture2DMipMap();
    mip->SizeX = tmip.SizeX;
    mip->SizeY = tmip.SizeY;

    void* data = malloc(tmip.Size);
    memcpy(data, tmip.Data, tmip.Size);
    mip->Data = new FByteBulkData(texture->GetPackage(), TEXTURE2D_COMPRESSION ? BULKDATA_SerializeCompressedLZO : BULKDATA_None, tmip.Size, data, true);
    texture->Mips.push_back(mip);
  }
  
  texture->MarkDirty();
  return true;
}