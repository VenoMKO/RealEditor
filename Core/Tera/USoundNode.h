#pragma once
#include "UObject.h"
#include <Utils/SoundTravaller.h>

class USoundNodeWave : public UObject {
public:
  DECL_UOBJ(USoundNodeWave, UObject);

  UPROP(float, Duration, 0.);
  UPROP(int32, NumChannels, 1);
  UPROP(int32, SampleRate, 0);

  bool RegisterProperty(FPropertyTag* property) override;

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
    free(ResourceData);
  }

  friend bool SoundTravaller::Visit(USoundNodeWave* texture);

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
