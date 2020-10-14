#pragma once
#include "UObject.h"

class USoundNodeWave : public UObject {
public:
  DECL_UOBJ(USoundNodeWave, UObject);

  void Serialize(FStream& s) override;
  void PostLoad() override;

  const void* GetResourceData() const
  {
    return ResourceData;
  }

  uint32 GetResourceSize() const
  {
    return ResourceSize;
  }

  ~USoundNodeWave() override
  {
    delete ResourceData;
  }

protected:
  FByteBulkData EditorData;
  FByteBulkData PCData;
  FByteBulkData XBoxData;
  FByteBulkData PS3Data;
  FByteBulkData UnkData1;
  FByteBulkData UnkData2;
  FByteBulkData UnkData3;
  FByteBulkData UnkData4;
  FByteBulkData UnkData5;

  void* ResourceData = nullptr;
  uint32 ResourceSize = 0;
};
