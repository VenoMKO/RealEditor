#include "UTexture.h"
#include "Cast.h"

void UTexture::RegisterProperty(FPropertyTag* property)
{
  if (PROP_IS(property, SRGB))
  {
    SRGB = property->BoolVal;
  }
}

void UTexture::Serialize(FStream& s)
{
  Super::Serialize(s);
  SourceArt.Serialize(s, this);
}

void UTexture::PostLoad()
{
  Super::PostLoad();
}

void FTexture2DMipMap::Serialize(FStream& s, UObject* owner, int32 mipIdx)
{
  Data.Serialize(s, owner, mipIdx);
  s << SizeX << SizeY;
}

UTexture2D::~UTexture2D()
{
  for (auto& e : Mips)
  {
    delete e;
  }
  for (auto& e : CachedMips)
  {
    delete e;
  }
  for (auto& e : CachedAtiMips)
  {
    delete e;
  }
  for (auto& e : CachedEtcMips)
  {
    delete e;
  }
}

void UTexture2D::RegisterProperty(FPropertyTag* property)
{
  if (PROP_IS(property, Format))
  {
    Format = (EPixelFormat)property->Value->GetByte();
  }
  else if (PROP_IS(property, SizeX))
  {
    SizeX = property->Value->GetInt();
  }
  else if (PROP_IS(property, SizeY))
  {
    SizeY = property->Value->GetInt();
  }
}

void UTexture2D::Serialize(FStream& s)
{
  Super::Serialize(s);
  if (s.GetFV() == VER_TERA_CLASSIC || s.GetFV() == VER_TERA_MODERN)
  {
    s << SourceFilePath;
  }
  s.SerializeUntypeBulkDataArray(Mips, this);
  s << TextureFileCacheGuid;
  if (s.GetFV() >= VER_TERA_CLASSIC)
  {
    s.SerializeUntypeBulkDataArray(CachedMips, this);
    s << MaxCachedResolution;
    s.SerializeUntypeBulkDataArray(CachedAtiMips, this);
    CachedFlashMip.Serialize(s, this);
    s.SerializeUntypeBulkDataArray(CachedEtcMips, this);
  }
}

void UTexture2D::PostLoad()
{
  Super::PostLoad();
}

