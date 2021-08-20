#include "UTexture.h"
#include "Cast.h"
#include "FPackage.h"
#include "UClass.h"

#include "Utils/ALog.h"
#include "Utils/TextureProcessor.h"

#include <../Extern/glew/glew.h>
#include <osg/Image>

struct PF_Info {
  uint32 BlockSizeX = 0;
  uint32 BlockSizeY = 0;
  uint32 BlockSizeZ = 0;
  uint32 BlockBytes = 0;
  uint32 NumComponents = 0;
  GLenum Format = 0;
};

PF_Info PixelFormatInfo[] = {
  //BsX     BsY     BsZ     Bb      Comps   GL_Format           //  Format
  { 0,			0,			0,			0,			0, 			GL_NONE},           //	PF_Unknown
  { 1,			1,			1,			16,			4, 			GL_RGBA},           //	PF_A32B32G32R32F
  { 1,			1,			1,			4,			4, 			GL_BGRA},           //	PF_A8R8G8B8
  { 1,			1,			1,			1,			1, 			GL_LUMINANCE},      //	PF_G8
  { 1,			1,			1,			2,			1, 			GL_NONE},           //	PF_G16
  { 4,			4,			1,			8,			3, 			GL_COMPRESSED_RGB_S3TC_DXT1_EXT},           //	PF_DXT1
  { 4,			4,			1,			16,			4, 			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT},           //	PF_DXT3
  { 4,			4,			1,			16,			4, 			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT},           //	PF_DXT5
  { 2,			1,			1,			4,			4, 			GL_NONE},           //	PF_UYVY
  { 1,			1,			1,			8,			3, 			GL_RGB},            //	PF_FloatRGB
  { 1,			1,			1,			8,			4, 			GL_RGBA},           //	PF_FloatRGBA
  { 1,			1,			1,			0,			1, 			GL_DEPTH_STENCIL},  //	PF_DepthStencil
  { 1,			1,			1,			4,			1, 			GL_DEPTH_COMPONENT},//	PF_ShadowDepth
  { 1,			1,			1,			4,			1, 			GL_DEPTH_COMPONENT},//	PF_FilteredShadowDepth
  { 1,			1,			1,			4,			1, 			GL_RED},            //	PF_R32F
  { 1,			1,			1,			4,			2, 			GL_RG},             //	PF_G16R16
  { 1,			1,			1,			4,			2, 			GL_RG},             //	PF_G16R16F
  { 1,			1,			1,			4,			2, 			GL_RG},             //	PF_G16R16F_FILTER
  { 1,			1,			1,			8,			2, 			GL_RG},             //	PF_G32R32F
  { 1,			1,			1,			4,			4, 			GL_RGBA},           //  PF_A2B10G10R10
  { 1,			1,			1,			8,			4, 			GL_RGBA},           //	PF_A16B16G16R16
  { 1,			1,			1,			4,			1, 			GL_NONE},           //	PF_D24
  { 1,			1,			1,			2,			1, 			GL_RED},            //	PF_R16F
  { 1,			1,			1,			2,			1, 			GL_RED},            //	PF_R16F_FILTER
  { 4,			4,			1,			16,			2, 			GL_NONE},           //	PF_BC5
  { 1,			1,			1,			2,			2, 			GL_RG},             //	PF_V8U8
  { 1,			1,			1,			1,			1, 			GL_NONE},           //	PF_A1			
  { 1,			1,			1,			0,			3, 			GL_NONE},           //	PF_FloatR11G11B10
};

inline bool FindInternalFormatAndType(uint32 pixelFormat, GLenum& outFormat, GLenum& outType, bool bSRGB)
{
  switch (pixelFormat)
  {
  case PF_A32B32G32R32F:
    outFormat = bSRGB ? GL_NONE : GL_RGBA32F_ARB;
    outType = GL_UNSIGNED_NORMALIZED_ARB;
    return true;
  case PF_A8R8G8B8:
    outFormat = bSRGB ? GL_SRGB8_ALPHA8_EXT : GL_RGBA;
    outType = GL_UNSIGNED_BYTE;
    return true;
  case PF_DXT1:
    outFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    outType = GL_UNSIGNED_BYTE;
    return true;
  case PF_DXT3:
    outFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    outType = GL_UNSIGNED_BYTE;
    return true;
  case PF_DXT5:
    outFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    outType = GL_UNSIGNED_BYTE;
    return true;
  case PF_G8:
    outFormat = bSRGB ? GL_RGB8 : GL_LUMINANCE8;
    outType = GL_UNSIGNED_BYTE;
    return true;
  case PF_DepthStencil:
    outFormat = bSRGB ? GL_NONE : GL_DEPTH24_STENCIL8_EXT;
    outType = GL_UNSIGNED_INT_24_8_EXT;
    return true;
  case PF_D24:
  case PF_ShadowDepth:
  case PF_FilteredShadowDepth:
    outFormat = bSRGB ? GL_NONE : GL_DEPTH_COMPONENT24_ARB;
    outType = GL_UNSIGNED_INT;
    return true;
  case PF_R32F:
    outFormat = bSRGB ? GL_NONE : GL_R32F;
    outType = GL_FLOAT;
    return true;
  case PF_G16R16:
    outFormat = bSRGB ? GL_RGBA16 : GL_RG16;
    outType = GL_UNSIGNED_SHORT;
    return true;
  case PF_G16R16F:
  case PF_G16R16F_FILTER:
    outFormat = bSRGB ? GL_RGBA16F_ARB : GL_RG16F;
    outType = GL_HALF_FLOAT_ARB;
    return true;
  case PF_G32R32F:
    outFormat = bSRGB ? GL_RGBA32F_ARB : GL_RG32F;
    outType = GL_FLOAT;
    return true;
  case PF_A2B10G10R10:
    outFormat = bSRGB ? GL_NONE : GL_RGB10_A2;
    outType = GL_UNSIGNED_INT_2_10_10_10_REV;
    return true;
  case PF_A16B16G16R16:
    outFormat = bSRGB ? GL_NONE : GL_RGBA16F_ARB;
    outType = GL_HALF_FLOAT_ARB;
    return true;
  case PF_R16F:
  case PF_R16F_FILTER:
    outFormat = bSRGB ? GL_NONE : GL_R16F;
    outType = GL_HALF_FLOAT_ARB;
    return true;
  case PF_FloatRGB:
    outFormat = bSRGB ? GL_NONE : GL_RGB16F_ARB;
    outType = GL_HALF_FLOAT_ARB;
    return true;
  case PF_FloatRGBA:
    outFormat = bSRGB ? GL_NONE : GL_RGBA16F_ARB;
    outType = GL_HALF_FLOAT_ARB;
    return true;
  case PF_V8U8:
    outFormat = bSRGB ? GL_NONE : GL_RG8;
    outType = GL_BYTE;
    return true;
  default:
    return false;
  }
}

bool UTexture::RegisterProperty(FPropertyTag* property)
{
  if (PROP_IS(property, SRGB))
  {
    SRGB = property->BoolVal;
    SRGBProperty = property;
    return true;
  }
  if (PROP_IS(property, CompressionSettings))
  {
    CompressionSettings = (TextureCompressionSettings)property->Value->GetByte();
    CompressionSettingsProperty = property;
    return true;
  }
  if (PROP_IS(property, UnpackMin))
  {
    UnpackMin[property->ArrayIndex] = property->GetFloat();
    UnpackMinProperty[property->ArrayIndex] = property;
    return true;
  }
  else if (PROP_IS(property, LODGroup))
  {
    LODGroup = (TextureGroup)property->Value->GetByte();
    LODGroupProperty = property;
    return true;
  }
  return false;
}

void UTexture::Serialize(FStream& s)
{
  Super::Serialize(s);
  SourceArt.Serialize(s, this);
}

UTexture2D::~UTexture2D()
{
  DeleteStorage();
}

bool UTexture2D::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return false;
  }
  if (PROP_IS(property, Format))
  {
    Format = (EPixelFormat)property->Value->GetByte();
    FormatProperty = property;
    return true;
  }
  else if (PROP_IS(property, SizeX))
  {
    SizeX = property->Value->GetInt();
    SizeXProperty = property;
    return true;
  }
  else if (PROP_IS(property, SizeY))
  {
    SizeY = property->Value->GetInt();
    SizeYProperty = property;
    return true;
  }
  else if (PROP_IS(property, AddressX))
  {
    AddressX = (TextureAddress)property->Value->GetByte();
    AddressXProperty = property;
    return true;
  }
  else if (PROP_IS(property, AddressY))
  {
    AddressY = (TextureAddress)property->Value->GetByte();
    AddressYProperty = property;
    return true;
  }
  else if (PROP_IS(property, MipTailBaseIdx))
  {
    MipTailBaseIdx = property->Value->GetInt();
    MipTailBaseIdxProperty = property;
    return true;
  }
  else if (PROP_IS(property, FirstResourceMemMip))
  {
    FirstResourceMemMip = property->Value->GetInt();
    FirstResourceMemMipProperty = property;
    return true;
  }
  else if (PROP_IS(property, bNoTiling))
  {
    bNoTiling = property->GetBool();
    bNoTilingProperty = property;
    return true;
  }
  else if (PROP_IS(property, TextureFileCacheName))
  {
    TextureFileCacheName = property->Value->GetNamePtr();
    TextureFileCacheNameProperty = property;
    //return true; Let subclasses get this value
  }
  else if (PROP_IS(property, NeverStream))
  {
    NeverStream = property->GetBool();
    NeverStreamProperty = property;
    return true;
  }
  else if (PROP_IS(property, bIsStreamable))
  {
    bIsStreamable = property->GetBool();
    bIsStreamableProperty = property;
    return true;
  }
  return false;
}

bool UTexture2D::RenderTo(osg::Image* target, int32 maxWidth, int32 maxHeight)
{
  GLenum type = 0;
  GLenum format = PixelFormatInfo[Format].Format;
  GLenum inFormat = format;
  if (!Mips.size() || !FindInternalFormatAndType(Format, inFormat, type, SRGB) || inFormat == GL_NONE || format == GL_NONE)
  {
    return false;
  }

  for (FTexture2DMipMap* mip : Mips)
  {
    if (mip->SizeX && mip->SizeY && mip->Data->GetAllocation())
    {
      if ((maxHeight && mip->SizeY > maxHeight) || (maxWidth && mip->SizeX > maxWidth))
      {
        continue;
      }
      target->setImage(mip->SizeX, mip->SizeY, 0, inFormat, format, type, (uint8*)mip->Data->GetAllocation(), osg::Image::AllocationMode::NO_DELETE);
      return true;
    }
  }

  return false;
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
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s.SerializeUntypeBulkDataArray(CachedMips, this);
    s << MaxCachedResolution;
    s.SerializeUntypeBulkDataArray(CachedAtiMips, this);
    CachedFlashMip.Serialize(s, this);
    s.SerializeUntypeBulkDataArray(CachedEtcMips, this);
  }
}

unsigned int UTexture2D::Hash() const
{
  if (!Loaded)
  {
    return 0;
  }
  for (FTexture2DMipMap* mip : Mips)
  {
    if (mip->Data && mip->Data->ElementCount)
    {
      return CalculateDataCRC(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize());
    }
  }
  return 0;
}

void UTexture2D::DisableCaching()
{
  Load();
  
  if (TextureFileCacheNameProperty)
  {
    RemoveProperty(TextureFileCacheNameProperty);
    TextureFileCacheName = nullptr;
    TextureFileCacheNameProperty = nullptr;
  }

  TextureFileCacheGuid = FGuid();

  if (FirstResourceMemMip != 0)
  {
    FirstResourceMemMip = 0;
    if (FirstResourceMemMipProperty)
    {
      FirstResourceMemMipProperty->Value->GetInt() = 0;
    }
  }

  if (MipTailBaseIdxProperty)
  {
    MipTailBaseIdx = 0;
    MipTailBaseIdxProperty->Value->GetInt() = 0;
  }

  if (NeverStreamProperty)
  {
    NeverStreamProperty->GetBool() = true;
    NeverStream = true;
  }
  else
  {
    CreatePropertyNeverStream(true);
  }

  if (bIsStreamableProperty)
  {
    bIsStreamableProperty->GetBool() = false;
    bIsStreamable = false;
  }
  else
  {
    CreatePropertybIsStreamable(false);
  }

  for (int32 idx = 0; idx < Mips.size(); ++idx)
  {
    if (idx > 0)
    {
      delete Mips[idx];
      continue;
    }
    FTexture2DMipMap* mip = Mips[idx];
    if (mip->Data->BulkDataFlags & BULKDATA_Unused)
    {
      continue;
    }
    if (mip->Data->IsStoredCompressedOnDisk())
    {
      // TODO: might be a different compression
      // TODO: remove this when switched to lzopro
      mip->Data->BulkDataFlags &= ~BULKDATA_SerializeCompressedLZO;
    }
    if (mip->Data->IsStoredInSeparateFile())
    {
      mip->Data->BulkDataFlags &= ~BULKDATA_StoreInSeparateFile;
    }
  }
  Mips.resize(1);
  MarkDirty();
}

void UTexture2D::PostLoad()
{
  Super::PostLoad();
  FString cacheName;
  FStream* rs = nullptr;
  for (int32 idx = 0; idx < Mips.size(); ++idx)
  {
    FTexture2DMipMap* mip = Mips[idx];
    if (mip->Data->IsStoredInSeparateFile())
    {
      bool loaded = false;
      if (TextureFileCacheName)
      {
        if (TextureFileCacheName->String() != cacheName)
        {
          if (rs)
          {
            delete rs;
            rs = nullptr;
          }
          FString path = FPackage::GetTextureFileCachePath(TextureFileCacheName->String());
          rs = new FReadStream(path);
          if (rs->IsGood())
          {
            cacheName = TextureFileCacheName->String();
          }
        }

        if (rs && rs->IsGood())
        {
          FStream& s = *rs;
          s.SetPosition(mip->Data->GetBulkDataOffsetInFile());

          try
          {
            mip->Data->SerializeSeparate(s, this, idx);
            loaded = true;
          }
          catch (...)
          {
            LogE("Failed to serialize a separate mip: %s.MipLevel_%d", GetObjectPath().C_str(), idx);
            delete rs;
            return;
          }
        }
      }

      if (loaded)
      {
        continue;
      }

      if (FPackage::GetCoreVersion() < VER_TERA_MODERN && (GetExportFlags() & EF_ForcedExport) && mip->Data->GetBulkDataOffsetInFile() >= 0)
      {
        FString objectPath = GetObjectPath();
        FString packageName;
        {
          std::vector<FString> comps = objectPath.Split('.');
          if (comps.size() > 2)
          {
            packageName = comps[1];
            objectPath = objectPath.Substr(comps[0].Size() + comps[1].Size() + 2);
          }
        }
        
        for (int32 number = 1; packageName.Size() && number < 6; ++number)
        {
          std::shared_ptr<FPackage> package = nullptr;
          try
          {
            FString pkgToOpen = packageName;
            if (number > 1)
            {
              pkgToOpen += "_" + std::to_string(number);
            }
            if (package = FPackage::GetPackageNamed(pkgToOpen))
            {
              if (package.get() == GetPackage())
              {
                FPackage::UnloadPackage(package);
                continue;
              }
              package->Load();
              if (UTexture2D* tex = Cast<UTexture2D>(package->GetObject(objectPath)))
              {
                FReadStream s = FReadStream(A2W(package->GetDataPath()));
                s.SetPackage(package.get());
                s.SetLoadSerializedObjects(package->GetStream().GetLoadSerializedObjects());
                s.SetPosition(mip->Data->GetBulkDataOffsetInFile());
                try
                {
                  mip->Data->SerializeSeparate(s, this, idx);
                  loaded = true;
                  GetPackage()->RetainPackage(package);
                }
                catch (...)
                {
                }
              }
              if (!loaded)
              {
                FPackage::UnloadPackage(package);
              }
            }
          }
          catch (...)
          {
          }
          if (loaded)
          {
            break;
          }
        }
      }

      if (loaded)
      {
        continue;
      }

      FString bulkDataName = GetObjectPath() + ".MipLevel_" + std::to_string(idx);
      bulkDataName = bulkDataName.ToUpper();
      FBulkDataInfo* info = FPackage::GetBulkDataInfo(bulkDataName);
      if (!info)
      {
        bulkDataName += "DXT";
        info = FPackage::GetBulkDataInfo(bulkDataName);
      }
      if (info)
      {
        if (cacheName != info->TextureFileCacheName)
        {
          if (rs)
          {
            delete rs;
            rs = nullptr;
          }
          FString path = FPackage::GetTextureFileCachePath(info->TextureFileCacheName);
          if (path.Size())
          {
            rs = new FReadStream(path);
            if (rs->IsGood())
            {
              cacheName = info->TextureFileCacheName;
            }
          }
        }

        if (rs)
        {
          FStream& s = *rs;
          s.SetPosition(info->SavedBulkDataOffsetInFile);
          try
          {
            mip->Data->SerializeSeparate(s, this, idx);
          }
          catch (...)
          {
            LogE("Failed to decompress bulkdata: %s", bulkDataName.C_str());
            delete rs;
            return;
          }
        }
      }
    }
  }
  if (rs)
  {
    delete rs;
  }
}

void UTexture2D::DeleteStorage()
{
  for (auto& e : Mips)
  {
    delete e;
  }
  Mips.clear();
  for (auto& e : CachedMips)
  {
    delete e;
  }
  CachedMips.clear();
  for (auto& e : CachedAtiMips)
  {
    delete e;
  }
  CachedAtiMips.clear();
  for (auto& e : CachedEtcMips)
  {
    delete e;
  }
  CachedEtcMips.clear();
}


EPixelFormat UTexture2D::GetEffectivePixelFormat(const EPixelFormat format, bool bSRGB)
{
  if ((format == PF_G8) && bSRGB)
  {
    return PF_A8R8G8B8;
  }
  return format;
}

bool UTextureCube::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(FacePosX, UTexture2D*);
  REGISTER_TOBJ_PROP(FaceNegX, UTexture2D*);
  REGISTER_TOBJ_PROP(FacePosY, UTexture2D*);
  REGISTER_TOBJ_PROP(FaceNegY, UTexture2D*);
  REGISTER_TOBJ_PROP(FacePosZ, UTexture2D*);
  REGISTER_TOBJ_PROP(FaceNegZ, UTexture2D*);
  return false;
}

bool UTextureCube::RenderTo(osg::Image* target)
{
  if (!HasAVX2())
  {
    return false;
  }
  std::array<UTexture2D*, 6> faces = GetFaces();
  TextureProcessor::TCFormat inputFormat = TextureProcessor::TCFormat::None;
  for (int32 idx = 0; idx < faces.size(); ++idx)
  {
    if (!faces[idx])
    {
      return false;
    }
    TextureProcessor::TCFormat f = TextureProcessor::TCFormat::None;
    switch (faces[idx]->Format)
    {
    case PF_DXT1:
      f = TextureProcessor::TCFormat::DXT1;
      break;
    case PF_DXT3:
      f = TextureProcessor::TCFormat::DXT3;
      break;
    case PF_DXT5:
      f = TextureProcessor::TCFormat::DXT5;
      break;
    case PF_A8R8G8B8:
      f = TextureProcessor::TCFormat::ARGB8;
      break;
    case PF_G8:
      f = TextureProcessor::TCFormat::G8;
      break;
    default:
      return false;
    }
    if (inputFormat != TextureProcessor::TCFormat::None && inputFormat != f)
    {
      return false;
    }
    inputFormat = f;
  }

  TextureProcessor processor(inputFormat, TextureProcessor::TCFormat::ARGB8);
  for (int32 idx = 0; idx < faces.size(); ++idx)
  {
    FTexture2DMipMap* mip = nullptr;
    for (FTexture2DMipMap* mipmap : faces[idx]->Mips)
    {
      if (mipmap->Data && mipmap->Data->GetAllocation() && mipmap->SizeX && mipmap->SizeY)
      {
        mip = mipmap;
        break;
      }
    }
    if (!mip)
    {
      LogE("Failed to export texture cube face %s.%s. No mipmaps!", GetObjectPath().UTF8().c_str(), faces[idx]->GetObjectNameString().UTF8().c_str());
      return false;
    }
    processor.SetInputCubeFace(idx, mip->Data->GetAllocation(), mip->Data->GetBulkDataSize(), mip->SizeX, mip->SizeY);
  }

  if (!processor.UnfoldCube())
  {
    LogE("Failed to render cube: %s", processor.GetError().c_str());
    return false;
  }

  GLenum type = 0;
  GLenum format = PixelFormatInfo[PF_A8R8G8B8].Format;
  GLenum inFormat = format;
  auto& mips = processor.GetOutputMips();

  if (!mips.size() || !FindInternalFormatAndType(PF_A8R8G8B8, inFormat, type, SRGB) || inFormat == GL_NONE || format == GL_NONE)
  {
    return false;
  }

  for (auto& mip : mips)
  {
    if (mip.SizeX && mip.SizeY && mip.Data)
    {
      void* data = malloc(mip.Size);
      memcpy(data, mip.Data, mip.Size);
      target->setImage(mip.SizeX, mip.SizeY, 0, inFormat, format, type, (uint8*)data, osg::Image::AllocationMode::USE_MALLOC_FREE);
      return true;
    }
  }
  return false;
}

void UTextureCube::PostLoad()
{
  Super::PostLoad();
  LoadObject(FacePosX);
  LoadObject(FaceNegX);
  LoadObject(FacePosY);
  LoadObject(FaceNegY);
  LoadObject(FacePosZ);
  LoadObject(FaceNegZ);
}

void ULightMapTexture2D::ConfigureClassObject(UClass* obj)
{
  obj->SetSuperStruct(obj->GetPackage()->GetClass(UTexture2D::StaticClassName()));
  obj->SetPropertyLink(obj->GetSuperStruct()->GetPropertyLink());
}

void ULightMapTexture2D::Serialize(FStream& s)
{
  if (s.IsReading())
  {
    LODGroup = TEXTUREGROUP_Lightmap;
  }
  Super::Serialize(s);
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << LightmapFlags;
  }
}
