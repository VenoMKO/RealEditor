#include "UPersistentCookerData.h"
#include "FObjectResource.h"
#include "FPackage.h"

#include <filesystem>

void UPersistentCookerData::GetPersistentData(std::unordered_map<FString, FBulkDataInfo>& outputBulk, std::unordered_map<FString, FTextureFileCacheInfo>& outputTFC)
{
  if (!IsLoaded())
  {
    FReadStream fs(GetPackage()->GetDataPath());
    fs.SetPackage(GetPackage());

    // Temporary sacrifice ~500Mb of RAM to get much lower load time
    void* rawData = malloc(Export->SerialSize);
    if (!rawData)
    {
      UThrow("Not enough RAM to load persistent data!");
    }
    fs.SerializeBytesAt(rawData, Export->SerialOffset, Export->SerialSize);

    MReadStream tfcStream(rawData, true, Export->SerialSize, Export->SerialOffset);
    tfcStream.SetPackage(GetPackage());
    FILE_OFFSET start = GetCookedBulkDataInfoMapOffset(tfcStream);
    tfcStream.SetPosition(start);

    // Both threads start from the same offset, but tfc thread won't serialize bulkData.
    // This allows to start TFC serialization while first thread is still serializing bulkData
    std::thread tfcThread([&outputTFC, &tfcStream] {
      int32 cnt = 0;
      tfcStream << cnt;
      for (int32 idx = 0; idx < cnt; ++idx)
      {
        int32 len = 0;
        tfcStream << len;
        tfcStream.SetPosition(tfcStream.GetPosition() + len + (sizeof(int32) * 6));
      }

      tfcStream << cnt;
      for (int32 idx = 0; idx < cnt; ++idx)
      {
        int32 len = 0;
        tfcStream << len;
        tfcStream.SetPosition(tfcStream.GetPosition() + len + sizeof(double));
      }

      uint64 textureFileCacheWaste = 0;
      tfcStream << textureFileCacheWaste;

      tfcStream << cnt;
      for (int32 idx = 0; idx < cnt; ++idx)
      {
        int32 len = 0;
        tfcStream << len;
        tfcStream.SetPosition(tfcStream.GetPosition() + len + sizeof(int32));
      }

      uint32 unk3 = 0;
      tfcStream << unk3;

      tfcStream << cnt;
      outputTFC.clear();
      outputTFC.reserve(cnt);
      FString key;
      FCookedTextureFileCacheInfo tmp;
      for (int32 idx = 0; idx < cnt; ++idx)
      {
        tfcStream << key;
        tfcStream << tmp;
        outputTFC.emplace(key, FTextureFileCacheInfo(tmp));
      }
      DBreakIf(!tfcStream.IsGood());
    });

    MReadStream bulkDataStream(rawData, false, Export->SerialSize, Export->SerialOffset);
    bulkDataStream.SetPackage(GetPackage());
    bulkDataStream.SetPosition(start);

    std::thread bulkDataThread([&outputBulk, &bulkDataStream] {
      int32 cnt = 0;
      bulkDataStream << cnt;
      outputBulk.clear();
      outputBulk.reserve(cnt);
      FString key;
      FCookedBulkDataInfo tmp;
      for (int32 idx = 0; idx < cnt; ++idx)
      {
        bulkDataStream << key;
        bulkDataStream << tmp;
        outputBulk.emplace(key, FBulkDataInfo(tmp));
      }
      DBreakIf(!bulkDataStream.IsGood());
    });

    tfcThread.join();
    bulkDataThread.join();
  }
  else
  {
    outputBulk.clear();
    outputBulk.reserve(CookedBulkDataInfoMap.size());
    for (const auto pair : CookedBulkDataInfoMap)
    {
      outputBulk.emplace(pair.first, FBulkDataInfo(pair.second));
    }
    outputTFC.clear();
    outputTFC.reserve(CookedTextureFileCacheInfoMap.size());
    for (const auto pair : CookedTextureFileCacheInfoMap)
    {
      outputTFC.emplace(pair.first, FTextureFileCacheInfo(pair.second));
    }
  }
}

void UPersistentCookerData::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ClassMap;
  s << LocalizationMap;
  s << Unk1 << Unk2;
  DBreakIf(Unk1 || Unk2);
  // TODO: CookedBulkDataInfoMap and CookedTextureFileCacheInfoMap are serialized at the app load time. Don't dup these maps
  s << CookedBulkDataInfoMap;
  s << FilenameToTimeMap;
  s << TextureFileCacheWaste;
  s << FilenameToCookedVersion;
  s << Unk3;
  DBreakIf(Unk3);
  s << CookedTextureFileCacheInfoMap;
  s << TextureUsageInfos;
  s << CookedPrefixCommonInfoMap;
  s << PMapForcedObjectsMap;
  s << FilenameToScriptSHA;
  // TODO: serialize cooked stats
}

FILE_OFFSET UPersistentCookerData::GetCookedBulkDataInfoMapOffset(FStream& s)
{
  if (!RawDataOffset)
  {
    s.SetPosition(Export->SerialOffset);
    FName noneProp;
    s << NetIndex;
    s << noneProp;
    if (noneProp.String() != NAME_None)
    {
      // UPersistentCookerData should not have any properties
      UThrow("Unexpected property during presistent data load.");
    }
    RawDataOffset = s.GetPosition();
  }
  s.SetPosition(RawDataOffset);

  int32 cnt = 0;
  int32 subCnt = 0;
  s << cnt;
  for (int32 i = 0; i < cnt; ++i)
  {
    int32 len = 0;
    s << len;
    s.SetPosition(s.GetPosition() + len);
    s << subCnt;
    for (int32 j = 0; j < subCnt; ++j)
    {
      s << len;
      s.SetPosition(s.GetPosition() + len);
      s << len;
      s.SetPosition(s.GetPosition() + len);
    }
  }

  s << cnt;
  for (int32 i = 0; i < cnt; ++i)
  {
    int32 len = 0;
    s << len;
    s.SetPosition(s.GetPosition() + len);
    s << subCnt;
    int32 subSubCnt = 0;
    for (int32 j = 0; j < subCnt; ++j)
    {
      s << len;
      s.SetPosition(s.GetPosition() + len);
      s << subSubCnt;
      for (int32 k = 0; k < subSubCnt; ++k)
      {
        s << len;
        s.SetPosition(s.GetPosition() + len);
        s << len;
        s.SetPosition(s.GetPosition() + len);
      }
    }
  }
  s << Unk1 << Unk2;
  DBreakIf(Unk1 || Unk2);
  return s.GetPosition();
}
