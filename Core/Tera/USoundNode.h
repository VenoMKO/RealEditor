#pragma once
#include "UObject.h"
#include <Utils/SoundTravaller.h>

class USoundNode : public UObject {
public:
  DECL_UOBJ(USoundNode, UObject);

  UPROP_NOINIT(std::vector<USoundNode*>, ChildNodes);

  bool RegisterProperty(FPropertyTag* property) override;

  virtual void GetWaves(std::vector<class USoundNodeWave*>& waves);
};

struct FAmbientSoundSlot
{
  class USoundNodeWave* Wave = nullptr;
  float PitchScale = 1.f;
  float VolumeScale = 1.f;
  float Weight = 1.f;
};

class USoundNodeAttenuation : public USoundNode {
public:
  DECL_UOBJ(USoundNodeAttenuation, USoundNode);

  UPROP(SoundDistanceModel, DistanceModel, ATTENUATION_Logarithmic);
  UPROP(bool, bSpatialize, true);
  UPROP(bool, bAttenuate, true);

  UPROP(bool, bAttenuateWithLowPassFilter, false); //x86
  UPROP(float, MinRadius, 400.f);
  UPROP(float, MaxRadius, 4000.f);
  UPROP(float, LPFMinRadius, 3000.f);
  UPROP(float, LPFMaxRadius, 6000.f);

  UPROP(bool, bAttenuateWithLPF, false); //x64
  UPROP(float, RadiusMin, 400.f);
  UPROP(float, RadiusMax, 4000.f);
  UPROP(float, LPFRadiusMin, 3000.f);
  UPROP(float, LPFRadiusMax, 6000.f);

  bool GetAttenuateWithLPF() const
  {
    return bAttenuateWithLowPassFilterProperty ? bAttenuateWithLowPassFilter : bAttenuateWithLPF;
  }

  float GetRadiusMin() const
  {
    return MinRadiusProperty ? MinRadius : RadiusMin;
  }

  float GetRadiusMax() const
  {
    return MaxRadiusProperty ? MaxRadius : RadiusMax;
  }

  float GetLPFRadiusMin() const
  {
    return LPFMinRadiusProperty ? LPFMinRadius : LPFRadiusMin;
  }

  float GetLPFRadiusMax() const
  {
    return LPFMaxRadiusProperty ? LPFMaxRadius : LPFRadiusMax;
  }

  bool RegisterProperty(FPropertyTag* property) override;

  void PostLoad() override;
};

class USoundNodeAmbient : public USoundNode {
public:
  DECL_UOBJ(USoundNodeAmbient, USoundNode);

  UPROP(SoundDistanceModel, DistanceModel, ATTENUATION_Logarithmic);
  UPROP(bool, bSpatialize, true);
  UPROP(bool, bAttenuate, true);

  UPROP(bool, bAttenuateWithLowPassFilter, false); //x86
  UPROP(float, MinRadius, 2000.f);
  UPROP(float, MaxRadius, 5000.f);
  UPROP(float, LPFMinRadius, 3500.f);
  UPROP(float, LPFMaxRadius, 7000.f);

  UPROP(bool, bAttenuateWithLPF, false); //x64
  UPROP(float, RadiusMin, 400.f);
  UPROP(float, RadiusMax, 4000.f);
  UPROP(float, LPFRadiusMin, 3000.f);
  UPROP(float, LPFRadiusMax, 6000.f);

  UPROP(float, PitchMin, 1.f);
  UPROP(float, PitchMax, 1.f);
  UPROP(float, VolumeMin, .7f);
  UPROP(float, VolumeMax, .7f);
  UPROP(USoundNodeWave*, Wave, nullptr);
  UPROP_NOINIT(std::vector<FAmbientSoundSlot>, SoundSlots);

  bool RegisterProperty(FPropertyTag* property) override;

  void PostLoad() override;

  void GetWaves(std::vector<class USoundNodeWave*>& waves) override;
};

class USoundNodeAmbientNonLoop : public USoundNodeAmbient {
public:
  DECL_UOBJ(USoundNodeAmbientNonLoop, USoundNodeAmbient);

  UPROP(float, DelayTime, 1.f);

  bool RegisterProperty(FPropertyTag* property) override;
};

class USoundNodeAmbientNonLoopToggle : public USoundNodeAmbientNonLoop {
public:
  DECL_UOBJ(USoundNodeAmbientNonLoopToggle, USoundNodeAmbientNonLoop);
};

class USoundNodeConcatenator : public USoundNode {
public:
  DECL_UOBJ(USoundNodeConcatenator, USoundNode);

  UPROP_NOINIT(std::vector<float>, InputVolume);

  bool RegisterProperty(FPropertyTag* property) override;
};

class USoundNodeLooping : public USoundNode {
public:
  DECL_UOBJ(USoundNodeLooping, USoundNode);
  
  UPROP(bool, bLoopIndefinitely, false);
  UPROP(float, LoopCountMin, 1000000.f);
  UPROP(float, LoopCountMax, 1000000.f);
  UPROP(float, LoopCount, 1000000.f);

  bool RegisterProperty(FPropertyTag* property) override;
};

class USoundNodeMixer : public USoundNode {
public:
  DECL_UOBJ(USoundNodeMixer, USoundNode);

  UPROP_NOINIT(std::vector<float>, InputVolume);

  bool RegisterProperty(FPropertyTag* property) override;
};

class USoundNodeWave : public USoundNode {
public:
  DECL_UOBJ(USoundNodeWave, USoundNode);

  UPROP(float, Duration, 0.f);
  UPROP(int32, NumChannels, 1);
  UPROP(int32, SampleRate, 0);

  bool RegisterProperty(FPropertyTag* property) override;

  void Serialize(FStream& s) override;
  void PostLoad() override;

  void GetWaves(std::vector<class USoundNodeWave*>& waves) override;

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

class USoundNodeModulator : public USoundNode {
public:
  DECL_UOBJ(USoundNodeModulator, USoundNode);

  UPROP(float, PitchMin, .95f);
  UPROP(float, PitchMax, 1.05f);
  UPROP(float, VolumeMin, .95f);
  UPROP(float, VolumeMax, 1.05f);

  UPROP(float, PitchModulation, 1.f);
  UPROP(float, VolumeModulation, 1.f);

  bool RegisterProperty(FPropertyTag* property) override;
};

class USoundNodeRandom : public USoundNode {
public:
  DECL_UOBJ(USoundNodeRandom, USoundNode);

  UPROP_NOINIT(std::vector<float>, Weights);
  UPROP(bool, bRandomizeWithoutReplacement, true);

  bool RegisterProperty(FPropertyTag* property) override;
};

class USoundCue : public USoundNode {
public:
  DECL_UOBJ(USoundCue, USoundNode);

  UPROP_NOINIT(FName, SoundGroup);
  UPROP(USoundNode*, FirstNode, nullptr);
  UPROP(float, Duration, 1.f);
  UPROP(float, VolumeMultiplier, 1.f);
  UPROP(float, PitchMultiplier, 1.f);
  UPROP(float, MaxAudibleDistance, 1.f);

  bool RegisterProperty(FPropertyTag* property) override;

  void GetWaves(std::vector<class USoundNodeWave*>& waves) override;
};