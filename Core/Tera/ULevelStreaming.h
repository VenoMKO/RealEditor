#pragma once
#include "UObject.h"
#include "UVolumes.h"
#include "ULevel.h"

class ULevelStreaming : public UObject {
public:
  DECL_UOBJ(ULevelStreaming, UObject);
  UPROP(FString, PackageName, NAME_None); // The real prop type FName. TODO: fix

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
  bool LoadLevelPackage(const FString& packageName);

  ULevel* Level = nullptr;
};

class ULevelStreamingAlwaysLoaded : public ULevelStreaming {
public:
  DECL_UOBJ(ULevelStreamingAlwaysLoaded, ULevelStreaming);
};

class ULevelStreamingDistance : public ULevelStreaming {
public:
  DECL_UOBJ(ULevelStreamingDistance, ULevelStreaming);
};

class ULevelStreamingKismet : public ULevelStreaming {
public:
  DECL_UOBJ(ULevelStreamingKismet, ULevelStreaming);
};

class ULevelStreamingPersistent : public ULevelStreaming {
public:
  DECL_UOBJ(ULevelStreamingPersistent, ULevelStreaming);
};

class US1LevelStreamingDistance : public ULevelStreaming {
public:
  DECL_UOBJ(US1LevelStreamingDistance, ULevelStreaming);
  UPROP(int32, ZoneNumberX, 0);
  UPROP(int32, ZoneNumberY, 0);

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};

class US1LevelStreamingBaseLevel : public US1LevelStreamingDistance {
public:
  DECL_UOBJ(US1LevelStreamingBaseLevel, US1LevelStreamingDistance);
};

class US1LevelStreamingSound : public US1LevelStreamingDistance {
public:
  DECL_UOBJ(US1LevelStreamingSound, US1LevelStreamingDistance);
};

class US1LevelStreamingSuperLow : public US1LevelStreamingDistance {
public:
  DECL_UOBJ(US1LevelStreamingSuperLow, US1LevelStreamingDistance);
};

class US1LevelStreamingVOID : public ULevelStreaming {
public:
  DECL_UOBJ(US1LevelStreamingVOID, ULevelStreaming);
};

class ULevelStreamingVolume : public UVolume {
public:
  DECL_UOBJ(ULevelStreamingVolume, UVolume);
  UPROP(std::vector<ULevelStreaming*>, StreamingLevels, {});

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};