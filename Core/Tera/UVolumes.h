#pragma once
#include "UActor.h"

class UBrush : public UActor {
public:
  DECL_UOBJ(UBrush, UActor);

  UPROP(class UModel*, Brush, nullptr);
  UPROP(class UBrushComponent*, BrushComponent, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;
};

class UVolume : public UBrush {
public:
  DECL_UOBJ(UVolume, UBrush);
};

class UBlockingVolume : public UVolume {
public:
  DECL_UOBJ(UBlockingVolume, UVolume);
};

class UAeroVolume : public UVolume {
public:
  DECL_UOBJ(UAeroVolume, UVolume);
};

class UAeroInnerVolume : public UVolume {
public:
  DECL_UOBJ(UAeroInnerVolume, UVolume);
};

struct FReverbVolumeSettings {
  ReverbPreset ReverbType = REVERB_Default;
  float Volume = .5f;
  float FadeTime = 2.f;
};

class UReverbVolume : public UVolume {
public:
  DECL_UOBJ(UReverbVolume, UVolume);

  UPROP_NOINIT(FReverbVolumeSettings, Settings);

  bool RegisterProperty(FPropertyTag* property) override;

  void PostLoad() override;
};

class US1WaterVolume : public UVolume {
public:
  DECL_UOBJ(US1WaterVolume, UVolume);
};

class US1MusicVolume : public UVolume {
public:
  DECL_UOBJ(US1MusicVolume, UVolume);
  UPROP_NOINIT(std::vector<class USoundCue*>, MusicList);

  bool RegisterProperty(FPropertyTag* property) override;

  std::vector<class USoundNodeWave*> GetAllWaves() const;
};