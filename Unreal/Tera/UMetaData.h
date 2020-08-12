#pragma once
#include "UObject.h"

class UMetaData : public UObject {
public:
  DECL_UOBJ(UMetaData, UObject);

  const std::map<UObject*, std::map<FName, FString>>& GetObjectMetaDataMap();

protected:
  void Serialize(FStream& s) override;

protected:
  std::map<UObject*, std::map<FName, FString>> ObjectMetaDataMap;
};