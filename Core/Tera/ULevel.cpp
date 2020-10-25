#include "ULevel.h"
#include "UClass.h"
#include "UProperty.h"

#include "FPackage.h"
#include "Cast.h"

FStream& operator<<(FStream& s, FGuidPair& p)
{
  return s << p.Guid << p.RefId;;
}

FStream& operator<<(FStream& s, FCoverIndexPair& p)
{
  return s << p.ActorRefItem << p.SlotIdx;
}

void ULevelBase::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, ActorsArrayOwner);
  s << Actors;
  s << URL;
}

std::vector<UActor*> ULevelBase::GetActors() const
{
  std::vector<UActor*> result;
  for (PACKAGE_INDEX idx : Actors)
  {
    result.push_back(Cast<UActor>(GetPackage()->GetObject(idx)));
  }
  return result;
}

void ULevel::ConfigureClassObject(UClass* object)
{
  CreateProperty("LightmapTotalSize", UFloatProperty::StaticClassName(), object);
  CreateProperty("ShadowmapTotalSize", UFloatProperty::StaticClassName(), object);

  object->Link();
}

void ULevel::Serialize(FStream& s)
{
  Super::Serialize(s);

  SERIALIZE_UREF(s, Model);
  s << ModelComponents;
  
  s << LevelSequences;
  
  s << TextureToInstancesMap;
  s << DynamicTextureInstances;

  s << ApexDestructionDataSize;
  if (s.IsReading() && ApexDestructionDataSize)
  {
    ApexDestructionData = malloc(ApexDestructionDataSize);
  }
  s.SerializeBytes(ApexDestructionData, ApexDestructionDataSize);
  
  s << CachedPhysBSPDataElementSize;
  s << CachedPhysBSPData;

  s << CachedPhysSMDataMap;
  s << CachedPhysSMDataStore;

  s << CachedPhysPerTriSMDataMap;
  s << CachedPhysPerTriSMDataStore;

  s << CachedPhysBSPDataVersion;
  s << CachedPhysSMDataVersion;

  s << ForceStreamTextures;

  s << CachedPhysConvexBSPData;
  s << CachedPhysConvexBSPVersion;
  
  SERIALIZE_UREF(s, NavListStart);
  SERIALIZE_UREF(s, NavListEnd);

  SERIALIZE_UREF(s, CoverListStart);
  SERIALIZE_UREF(s, CoverListEnd);

  SERIALIZE_UREF(s, PylonListStart);
  SERIALIZE_UREF(s, PylonListEnd);

  s << CrossLevelCoverGuidRefs;

  s << CoverLinkRefs;
  s << CoverIndexPairs;
  s << CrossLevelActors;

  s << PrecomputedLightVolume;
  s << PrecomputedVisibilityHandler;
  s << PrecomputedVolumeDistanceField;

  s << Unk1;
  DBreakIf(Unk1);
}