#pragma once
#include "UObject.h"
#include "FStructs.h"

class UPersistentCookerData : public UObject {
public:
  DECL_UOBJ(UPersistentCookerData, UObject);

  // Load only needed maps without object serialization
  void GetPersistentData(std::unordered_map<FString, FBulkDataInfo>& outputBulk, std::unordered_map<FString, FTextureFileCacheInfo>& outputTFC);

  void Serialize(FStream& s) override;

protected:
  FILE_OFFSET GetCookedBulkDataInfoMapOffset(FStream& s);

private:
  std::unordered_map<FString, std::vector<FPacakgeTreeEntry>> ClassMap;
  std::unordered_map<FString, std::map<FString, std::vector<FPacakgeTreeEntry>>> LocalizationMap;
  uint32 Unk1 = 0;
  uint32 Unk2 = 0;
  uint32 Unk3 = 0;
  std::unordered_map<FString, FCookedBulkDataInfo> CookedBulkDataInfoMap;
  std::unordered_map<FString, FCookedTextureFileCacheInfo> CookedTextureFileCacheInfoMap;
  std::unordered_map<FString, double> FilenameToTimeMap;
  std::unordered_map<FString, int32> FilenameToCookedVersion;
  std::unordered_map<FString, FCookedTextureUsageInfo> TextureUsageInfos;
  std::unordered_map<FString, FForceCookedInfo> CookedPrefixCommonInfoMap;
  std::unordered_map<FString, FForceCookedInfo> PMapForcedObjectsMap;
  std::unordered_map<FString, FSHA> FilenameToScriptSHA;
  uint64 TextureFileCacheWaste = 0;
};