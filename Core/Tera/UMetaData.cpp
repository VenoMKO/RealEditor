#include "UMetaData.h"
#include "FObjectResource.h"
#include "FPackage.h"

#include <filesystem>

const std::map<UObject*, std::map<FName, FString>>& UMetaData::GetObjectMetaDataMap()
{
  return ObjectMetaDataMap;
}

void UMetaData::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ObjectMetaDataMap;
}
