#include "TfcBuilder.h"
#include <Tera/ALog.h>
#include <Tera/FPackage.h>
#include <Tera/UTexture.h>

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
    if (!tex->TextureFileCacheName)
    {
      continue;
    }

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
            FILE_OFFSET start = s.GetPosition();
            s.SerializeCompressed(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize(), mip->Data->GetDecompressionFlags());
            offsets[midx] = start;
            sizes[midx] = std::make_pair(s.GetPosition() - start, mip->Data->ElementCount);
            flags[midx] = mip->Data->BulkDataFlags;
          }
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
          continue;
        }
        tex->TextureFileCacheName->SetString(Name);
        for (int32 midx = 0; midx < tex->Mips.size(); ++midx)
        {
          FTexture2DMipMap* mip = tex->Mips[midx];
          if (mip->Data && mip->Data->IsStoredInSeparateFile() && offsets.count(midx))
          {
            mip->Data->BulkDataOffsetInFile = offsets[midx];
            mip->Data->BulkDataSizeOnDisk = sizes[midx].first;
            mip->Data->ElementCount = sizes[midx].second;
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
