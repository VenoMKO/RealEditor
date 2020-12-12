#pragma once
#include "UActor.h"
#include "UPhysAsset.h"

struct FGuidPair {
  FGuid	Guid;
  uint32 RefId = 0;

  friend FStream& operator<<(FStream& s, FGuidPair& p);
};

struct FCoverIndexPair {
  uint32 ActorRefItem = 0;
  uint8 SlotIdx = 0;

  friend FStream& operator<<(FStream& s, FCoverIndexPair& p);
};

class ULevelBase : public UObject {
public:
  DECL_UOBJ(ULevelBase, UObject);

  void Serialize(FStream& s) override;

  std::vector<UActor*> GetActors() const;

  int GetActorsCount() const
  {
    return (int)Actors.size();
  }

protected:
  DECL_UREF(UObject, ActorsArrayOwner);
  std::vector<PACKAGE_INDEX> Actors;
  FURL URL;
};

class ULevel : public ULevelBase {
public:
  DECL_UOBJ(ULevel, ULevelBase);

  static void ConfigureClassObject(UClass* object);

  void Serialize(FStream& s) override;

protected:
  DECL_UREF(UObject, Model);
  std::vector<PACKAGE_INDEX> ModelComponents;
  std::vector<PACKAGE_INDEX> LevelSequences;
  std::map<PACKAGE_INDEX, std::vector<FStreamableTextureInstance>> TextureToInstancesMap; // UTexture2D*
  std::map<PACKAGE_INDEX, std::vector<FDynamicTextureInstance>>	DynamicTextureInstances; // UPrimitiveComponent*
  void* ApexDestructionData = nullptr;
  FILE_OFFSET ApexDestructionDataSize = 0;
  std::vector<uint8> CachedPhysBSPData;
  FILE_OFFSET CachedPhysBSPDataElementSize = 1;
  std::multimap<PACKAGE_INDEX, FCachedPhysSMData> CachedPhysSMDataMap;
  std::vector<FKCachedConvexData> CachedPhysSMDataStore;
  std::multimap<PACKAGE_INDEX, FCachedPerTriPhysSMData> CachedPhysPerTriSMDataMap;
  std::vector<FKCachedPerTriData> CachedPhysPerTriSMDataStore;
  int32 CachedPhysSMDataVersion = 0;
  int32 CachedPhysBSPDataVersion = 0;
  std::map<PACKAGE_INDEX, bool> ForceStreamTextures;
  FKCachedConvexData CachedPhysConvexBSPData;
  int32 CachedPhysConvexBSPVersion = 0;
  DECL_UREF(UObject, NavListStart);
  DECL_UREF(UObject, NavListEnd); 
  DECL_UREF(UObject, CoverListStart);
  DECL_UREF(UObject, CoverListEnd);
  DECL_UREF(UObject, PylonListStart);
  DECL_UREF(UObject, PylonListEnd);
  std::vector<FGuidPair> CrossLevelCoverGuidRefs;
  std::vector<PACKAGE_INDEX> CoverLinkRefs;
  std::vector<FCoverIndexPair> CoverIndexPairs;
  std::vector<PACKAGE_INDEX> CrossLevelActors;
  FPrecomputedLightVolume PrecomputedLightVolume;
  FPrecomputedVisibilityHandler PrecomputedVisibilityHandler;
  FPrecomputedVolumeDistanceField PrecomputedVolumeDistanceField;
  int32 Unk1 = 0;
};