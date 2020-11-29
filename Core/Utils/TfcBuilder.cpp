#include "TfcBuilder.h"
#include <Tera/ALog.h>
#include <Tera/FObjectResource.h>
#include <Tera/FPackage.h>
#include <Tera/UTexture.h>
#include <Tera/UClass.h>

bool TfcBuilder::AddTexture(UTexture2D* texture)
{
  if (!texture)
  {
    return false;
  }
  for (UTexture2D* tex : Textures)
  {
    if (tex == texture)
    {
      return false;
    }
  }
  Textures.push_back(texture);
  return true;
}

bool TfcBuilder::Compile()
{
  if (Textures.empty())
  {
    Error = "Nothing to do!";
    return false;
  }
  std::map<uint32, std::vector<UTexture2D*>> tfcMap;
  for (UTexture2D* tex : Textures)
  {
    tex->Load();
    if (tex->NeverStream || tex->Mips.size() < 2 || tex->SizeX <= 64 || tex->SizeY <= 64)
    {
      continue;
    }
    if (uint32 crc = tex->Hash())
    {
      tfcMap[crc].push_back(tex);
    }
    else
    {
      LogW("Failed to calculate texture hash: %s", tex->GetObjectName().UTF8().c_str());
    }
  }

  if (tfcMap.empty())
  {
    Error = "Failed to calculate CRCs. Nothing to do!";
    return false;
  }

  MWrightStream s(nullptr, 0);
  for (const auto& p : tfcMap)
  {
    UTexture2D* tex = p.second.front();
    std::map<int32, FILE_OFFSET> offsets;
    std::map<int32, std::pair<FILE_OFFSET, FILE_OFFSET>> sizes;
    std::map<int32, uint32> flags;

    if (tex->TextureFileCacheName)
    {
      FReadStream rs(FPackage::GetTextureFileCachePath(tex->TextureFileCacheName->String()));
      if (rs.IsGood())
      {
        for (int32 midx = 0; midx < tex->Mips.size(); ++midx)
        {
          FTexture2DMipMap* mip = tex->Mips[midx];
          if (mip->Data && mip->Data->IsStoredInSeparateFile())
          {
            rs.SetPosition(mip->Data->GetBulkDataOffsetInFile());
            FILE_OFFSET tmpSize = mip->Data->GetBulkDataSizeOnDisk();
            void* tmp = malloc(tmpSize);
            rs.SerializeBytes(tmp, tmpSize);
            FILE_OFFSET start = s.GetPosition();
            s.SerializeBytes(tmp, tmpSize);
            free(tmp);
            offsets[midx] = start;
            sizes[midx] = std::make_pair(s.GetPosition() - start, mip->Data->ElementCount);
            flags[midx] = mip->Data->BulkDataFlags;
          }
        }
      }
      else
      {
        LogE("Failed to open %s", tex->TextureFileCacheName->String().UTF8().c_str());
      }
    }
    else
    {
      for (int32 midx = 0; midx < tex->Mips.size(); ++midx)
      {
        FTexture2DMipMap* mip = tex->Mips[midx];
        if (mip->SizeX > 64 && mip->SizeY > 64)
        {
          FILE_OFFSET start = s.GetPosition();
          s.SerializeCompressed(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize(), COMPRESS_LZO);
          mip->Data->BulkDataFlags &= ~BULKDATA_SerializeCompressed;
          mip->Data->BulkDataFlags |= (BULKDATA_SerializeCompressedLZO | BULKDATA_StoreInSeparateFile);

          offsets[midx] = start;
          sizes[midx] = std::make_pair(s.GetPosition() - start, mip->Data->ElementCount);
          flags[midx] = mip->Data->BulkDataFlags;
        }
        else if (offsets.size())
        {
          mip->Data->BulkDataFlags &= ~(BULKDATA_StoreInSeparateFile | BULKDATA_SerializeCompressed);
          mip->Data->BulkDataFlags |= BULKDATA_SerializeCompressedLZO;
        }
      }
    }

    if (offsets.size())
    {
      for (int32 idx = 0; idx < p.second.size(); ++idx)
      {
        UTexture2D* tex = p.second[idx];

        if (!tex->TextureFileCacheName)
        {
          tex->TextureFileCacheNameProperty = new FPropertyTag(tex, tex->P_TextureFileCacheName, NAME_NameProperty);
          tex->TextureFileCacheNameProperty->ClassProperty = tex->GetClass()->GetProperty(tex->P_TextureFileCacheName);
          tex->TextureFileCacheNameProperty->Value->Type = FPropertyValue::VID::Name;
          tex->TextureFileCacheNameProperty->Value->Data = new FName(tex->GetPackage(), Name);
          tex->TextureFileCacheName = tex->TextureFileCacheNameProperty->Value->GetNamePtr();
          tex->AddProperty(tex->TextureFileCacheNameProperty);
        }

        if (!tex->FirstResourceMemMipProperty)
        {
          tex->FirstResourceMemMipProperty = new FPropertyTag(tex, tex->P_FirstResourceMemMip, NAME_IntProperty);
          tex->FirstResourceMemMipProperty->ClassProperty = tex->GetClass()->GetProperty(tex->P_FirstResourceMemMip);
          tex->FirstResourceMemMipProperty->Value->Type = FPropertyValue::VID::Int;
          tex->FirstResourceMemMipProperty->Value->Data = new int32(offsets.size());
          tex->FirstResourceMemMip = tex->FirstResourceMemMipProperty->GetInt();
          tex->AddProperty(tex->FirstResourceMemMipProperty);
        }
        else if (tex->FirstResourceMemMip != offsets.size())
        {
          tex->FirstResourceMemMipProperty->GetInt() = int32(offsets.size());
          tex->FirstResourceMemMip = tex->FirstResourceMemMipProperty->GetInt();
        }

        tex->GetExportObject()->ExportFlags = EF_None;
        tex->TextureFileCacheName->SetString(Name);
        for (int32 midx = 0; midx < tex->Mips.size(); ++midx)
        {
          FTexture2DMipMap* mip = tex->Mips[midx];
          if (mip->Data && offsets.count(midx))
          {
            mip->Data->BulkDataOffsetInFile = offsets[midx];
            mip->Data->BulkDataSizeOnDisk = sizes[midx].first;
            mip->Data->ElementCount = sizes[midx].second;
          }
          if (mip->Data && flags.count(midx))
          {
            mip->Data->BulkDataFlags = flags[midx];
          }
        }

        tex->MarkDirty();
      }
    }
  }

  s.GetAllocation();
  if (TfcData)
  {
    free(TfcData);
    TfcData = 0;
    TfcDataSize = 0;
  }

  if (s.GetAllocation() && s.GetPosition())
  {
    TfcDataSize = s.GetPosition();
    TfcData = malloc(TfcDataSize);
    memcpy(TfcData, s.GetAllocation(), TfcDataSize);
    return true;
  }
  return false;
}
