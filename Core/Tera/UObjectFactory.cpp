#include "UObject.h"
#include "FObjectResource.h"
#include "FName.h"

#include "UClass.h"
#include "UProperty.h"
#include "UMetaData.h"

// Components
#include "UComponent.h"

// Objects
#include "UObjectRedirector.h"
#include "UMaterial.h"
#include "UMaterialExpression.h"
#include "UPersistentCookerData.h"
#include "UTexture.h"
#include "USkeletalMesh.h"
#include "UStaticMesh.h"
#include "USoundNode.h"
#include "UAnimation.h"
#include "UPhysAsset.h"
#include "ULevelStreaming.h"
#include "USpeedTree.h"
#include "UPrefab.h"
#include "UObjectReferencer.h"
#include "UModel.h"

#include "UActor.h"
#include "ULevel.h"
#include "UTerrain.h"
#include "ULight.h"

UObject* UObject::Object(FObjectExport* exp)
{
  const FString c = exp->GetClassNameString();
  if (c == UClass::StaticClassName())
  {
    return new UClass(exp, false);
  }
  if (c == UPackage::StaticClassName())
  {
    return new UPackage(exp);
  }
  if (c == UTexture2D::StaticClassName())
  {
    return new UTexture2D(exp);
  }
  if (c == UTextureCube::StaticClassName())
  {
    return new UTextureCube(exp);
  }
  if (c == UTextureFlipBook::StaticClassName())
  {
    return new UTextureFlipBook(exp);
  }
  if (c == UShadowMapTexture2D::StaticClassName())
  {
    return new UShadowMapTexture2D(exp);
  }
  if (c == ULightMapTexture2D::StaticClassName())
  {
    return new ULightMapTexture2D(exp);
  }
  if (c == USkeletalMesh::StaticClassName())
  {
    return new USkeletalMesh(exp);
  }
  if (c == UMaterial::StaticClassName())
  {
    return new UMaterial(exp);
  }
  if (c == UMaterialInstance::StaticClassName())
  {
    return new UMaterialInstance(exp);
  }
  if (c == UMaterialInstanceConstant::StaticClassName())
  {
    return new UMaterialInstanceConstant(exp);
  }
  if (c == UStaticMesh::StaticClassName())
  {
    return new UStaticMesh(exp);
  }
  if (c == URB_BodySetup::StaticClassName())
  {
    return new URB_BodySetup(exp);
  }
  if (c == UPhysicsAssetInstance::StaticClassName())
  {
    return new UPhysicsAssetInstance(exp);
  }
  if (c == USpeedTree::StaticClassName())
  {
    return new USpeedTree(exp);
  }
  if (c == UPrefab::StaticClassName())
  {
    return new UPrefab(exp);
  }
  if (c == UModel::StaticClassName())
  {
    return new UModel(exp);
  }
  if (c == UPolys::StaticClassName())
  {
    return new UPolys(exp);
  }
  if (c == UAeroVolume::StaticClassName())
  {
    return new UAeroVolume(exp);
  }
  if (c == UAeroInnerVolume::StaticClassName())
  {
    return new UAeroInnerVolume(exp);
  }
  if (c == UReverbVolume::StaticClassName())
  {
    return new UReverbVolume(exp);
  }
  if (c == US1MusicVolume::StaticClassName())
  {
    return new US1MusicVolume(exp);
  }
  if (c == UActor::StaticClassName())
  {
    return new UActor(exp);
  }
  if (c == UTerrain::StaticClassName())
  {
    return new UTerrain(exp);
  }
  if (c == UTerrainWeightMapTexture::StaticClassName())
  {
    return new UTerrainWeightMapTexture(exp);
  }
  if (c == UTerrainMaterial::StaticClassName())
  {
    return new UTerrainMaterial(exp);
  }
  if (c == UTerrainLayerSetup::StaticClassName())
  {
    return new UTerrainLayerSetup(exp);
  }
  if (c == UBrush::StaticClassName())
  {
    return new UBrush(exp);
  }
  if (c == UBrushComponent::StaticClassName())
  {
    return new UBrushComponent(exp);
  }
  if (c == ULevel::StaticClassName())
  {
    return new ULevel(exp);
  }
  if (c == ULevelStreamingAlwaysLoaded::StaticClassName())
  {
    return new ULevelStreamingAlwaysLoaded(exp);
  }
  if (c == ULevelStreamingDistance::StaticClassName())
  {
    return new ULevelStreamingDistance(exp);
  }
  if (c == ULevelStreamingKismet::StaticClassName())
  {
    return new ULevelStreamingKismet(exp);
  }
  if (c == ULevelStreamingPersistent::StaticClassName())
  {
    return new ULevelStreamingPersistent(exp);
  }
  if (c == US1LevelStreamingDistance::StaticClassName())
  {
    return new US1LevelStreamingDistance(exp);
  }
  if (c == US1LevelStreamingBaseLevel::StaticClassName())
  {
    return new US1LevelStreamingBaseLevel(exp);
  }
  if (c == US1LevelStreamingSound::StaticClassName())
  {
    return new US1LevelStreamingSound(exp);
  }
  if (c == US1LevelStreamingSuperLow::StaticClassName())
  {
    return new US1LevelStreamingSuperLow(exp);
  }
  if (c == US1LevelStreamingVOID::StaticClassName())
  {
    return new US1LevelStreamingVOID(exp);
  }
  if (c == US1WaterVolume::StaticClassName())
  {
    return new US1WaterVolume(exp);
  }
  if (c == ULevelStreamingVolume::StaticClassName())
  {
    return new ULevelStreamingVolume(exp);
  }
  if (c == UBlockingVolume::StaticClassName())
  {
    return new UBlockingVolume(exp);
  }
  if (c == UStaticMeshActor::StaticClassName())
  {
    return new UStaticMeshActor(exp);
  }
  if (c == USkeletalMeshActor::StaticClassName())
  {
    return new USkeletalMeshActor(exp);
  }
  if (c == USpeedTreeActor::StaticClassName())
  {
    return new USpeedTreeActor(exp);
  }
  if (c == ULight::StaticClassName())
  {
    return new ULight(exp);
  }
  if (c == UPointLight::StaticClassName())
  {
    return new UPointLight(exp);
  }
  if (c == UPointLightMovable::StaticClassName())
  {
    return new UPointLightMovable(exp);
  }
  if (c == UPointLightToggleable::StaticClassName())
  {
    return new UPointLightToggleable(exp);
  }
  if (c == USpotLight::StaticClassName())
  {
    return new USpotLight(exp);
  }
  if (c == USpotLightMovable::StaticClassName())
  {
    return new USpotLightMovable(exp);
  }
  if (c == USpotLightToggleable::StaticClassName())
  {
    return new USpotLightToggleable(exp);
  }
  if (c == UDirectionalLight::StaticClassName())
  {
    return new UDirectionalLight(exp);
  }
  if (c == UDirectionalLightToggleable::StaticClassName())
  {
    return new UDirectionalLightToggleable(exp);
  }
  if (c == USkyLight::StaticClassName())
  {
    return new USkyLight(exp);
  }
  if (c == USkyLightToggleable::StaticClassName())
  {
    return new USkyLightToggleable(exp);
  }
  if (c == UInterpActor::StaticClassName())
  {
    return new UInterpActor(exp);
  }
  if (c == UEmitter::StaticClassName())
  {
    return new UEmitter(exp);
  }
  if (c == UHeightFog::StaticClassName())
  {
    return new UHeightFog(exp);
  }
  if (c == UPrefabInstance::StaticClassName())
  {
    return new UPrefabInstance(exp);
  }
  if (c == UAnimSet::StaticClassName())
  {
    return new UAnimSet(exp);
  }
  if (c == UAnimSequence::StaticClassName())
  {
    return new UAnimSequence(exp);
  }
  if (c == USoundNodeWave::StaticClassName())
  {
    return new USoundNodeWave(exp);
  }
  if (c == USoundNodeAmbient::StaticClassName())
  {
    return new USoundNodeAmbient(exp);
  }
  if (c == USoundNodeAttenuation::StaticClassName())
  {
    return new USoundNodeAttenuation(exp);
  }
  if (c == USoundNodeAmbientNonLoop::StaticClassName())
  {
    return new USoundNodeAmbientNonLoop(exp);
  }
  if (c == USoundNodeAmbientNonLoopToggle::StaticClassName())
  {
    return new USoundNodeAmbientNonLoopToggle(exp);
  }
  if (c == USoundNodeConcatenator::StaticClassName())
  {
    return new USoundNodeConcatenator(exp);
  }
  if (c == USoundNodeLooping::StaticClassName())
  {
    return new USoundNodeLooping(exp);
  }
  if (c == USoundNodeMixer::StaticClassName())
  {
    return new USoundNodeMixer(exp);
  }
  if (c == USoundNodeModulator::StaticClassName())
  {
    return new USoundNodeModulator(exp);
  }
  if (c == USoundNodeRandom::StaticClassName())
  {
    return new USoundNodeRandom(exp);
  }
  if (c == USoundCue::StaticClassName())
  {
    return new USoundCue(exp);
  }
  if (c == UField::StaticClassName())
  {
    return new UField(exp);
  }
  if (c == UStruct::StaticClassName())
  {
    return new UStruct(exp);
  }
  if (c == UScriptStruct::StaticClassName())
  {
    return new UScriptStruct(exp);
  }
  if (c == UState::StaticClassName())
  {
    return new UState(exp);
  }
  if (c == UEnum::StaticClassName())
  {
    return new UEnum(exp);
  }
  if (c == UConst::StaticClassName())
  {
    return new UConst(exp);
  }
  if (c == UFunction::StaticClassName())
  {
    return new UFunction(exp);
  }
  if (c == UTextBuffer::StaticClassName())
  {
    return new UTextBuffer(exp);
  }
  if (c == UIntProperty::StaticClassName())
  {
    return new UIntProperty(exp);
  }
  if (c == UBoolProperty::StaticClassName())
  {
    return new UBoolProperty(exp);
  }
  if (c == UByteProperty::StaticClassName())
  {
    return new UByteProperty(exp);
  }
  if (c == UFloatProperty::StaticClassName())
  {
    return new UFloatProperty(exp);
  }
  if (c == UObjectProperty::StaticClassName())
  {
    return new UObjectProperty(exp);
  }
  if (c == UClassProperty::StaticClassName())
  {
    return new UClassProperty(exp);
  }
  if (c == UComponentProperty::StaticClassName())
  {
    return new UComponentProperty(exp);
  }
  if (c == UNameProperty::StaticClassName())
  {
    return new UNameProperty(exp);
  }
  if (c == UStrProperty::StaticClassName())
  {
    return new UStrProperty(exp);
  }
  if (c == UStructProperty::StaticClassName())
  {
    return new UStructProperty(exp);
  }
  if (c == UArrayProperty::StaticClassName())
  {
    return new UArrayProperty(exp);
  }
  if (c == UMapProperty::StaticClassName())
  {
    return new UMapProperty(exp);
  }
  if (c == UInterfaceProperty::StaticClassName())
  {
    return new UInterfaceProperty(exp);
  }
  if (c == UDelegateProperty::StaticClassName())
  {
    return new UDelegateProperty(exp);
  }
  if (c == UMetaData::StaticClassName())
  {
    return new UMetaData(exp);
  }
  if (c == UObjectRedirector::StaticClassName())
  {
    return new UObjectRedirector(exp);
  }
  if (c == UObjectReferencer::StaticClassName())
  {
    return new UObjectReferencer(exp);
  }
  if (c == UPersistentCookerData::StaticClassName())
  {
    return new UPersistentCookerData(exp);
  }
  if (c == UDynamicSMActor::StaticClassName())
  {
    return new UDynamicSMActor(exp);
  }
  if (c == UComponent::StaticClassName())
  {
    return new UComponent(exp);
  }
  if (c == UStaticMeshComponent::StaticClassName())
  {
    return new UStaticMeshComponent(exp);
  }
  if (c == USkeletalMeshComponent::StaticClassName())
  {
    return new USkeletalMeshComponent(exp);
  }
  if (c == USpeedTreeComponent::StaticClassName())
  {
    return new USpeedTreeComponent(exp);
  }
  if (c == ULightComponent::StaticClassName())
  {
    return new ULightComponent(exp);
  }
  if (c == UPointLightComponent::StaticClassName())
  {
    return new UPointLightComponent(exp);
  }
  if (c == USpotLightComponent::StaticClassName())
  {
    return new USpotLightComponent(exp);
  }
  if (c == UDirectionalLightComponent::StaticClassName())
  {
    return new UDirectionalLightComponent(exp);
  }
  if (c == UDominantDirectionalLightComponent::StaticClassName())
  {
    return new UDominantDirectionalLightComponent(exp);
  }
  if (c == UDominantSpotLightComponent::StaticClassName())
  {
    return new UDominantSpotLightComponent(exp);
  }
  if (c == USkyLightComponent::StaticClassName())
  {
    return new USkyLightComponent(exp);
  }
  if (c == UHeightFogComponent::StaticClassName())
  {
    return new UHeightFogComponent(exp);
  }
  if (c == UAudioComponent::StaticClassName())
  {
    return new UAudioComponent(exp);
  }
  if (c == UAmbientSound::StaticClassName())
  {
    return new UAmbientSound(exp);
  }
  if (c == UAmbientSoundMovable::StaticClassName())
  {
    return new UAmbientSoundMovable(exp);
  }
  if (c == UAmbientSoundSimple::StaticClassName())
  {
    return new UAmbientSoundSimple(exp);
  }
  if (c == UAmbientSoundNonLoop::StaticClassName())
  {
    return new UAmbientSoundNonLoop(exp);
  }
  if (c == UAmbientSoundSimpleToggleable::StaticClassName())
  {
    return new UAmbientSoundSimpleToggleable(exp);
  }
  if (c == UAmbientSoundNonLoopingToggleable::StaticClassName())
  {
    return new UAmbientSoundNonLoopingToggleable(exp);
  }
  if (c == UDistributionFloatConstant::StaticClassName())
  {
    return new UDistributionFloatConstant(exp);
  }
  if (c == UDistributionFloatUniform::StaticClassName())
  {
    return new UDistributionFloatUniform(exp);
  }
  
  // A fallback for unimplemented classes
  // MaterialExpressions must be before the UComponent due to UMaterialExpressionComponentMask
  if (c.StartWith("MaterialExpression"))
  {
    return UMaterialExpression::StaticFactory(exp);
  }
  // Fallback for unimplemented components. *Component => UComponent
  if ((c.Find(UComponent::StaticClassName()) != std::string::npos) ||
      (c.Find("Distribution") != std::string::npos) ||
       c.Find("UIComp_") != std::string::npos ||
       c == "RB_Handle")
  {
    return new UComponent(exp);
  }
  // Fallback for all *Actor classes except components
  if (c.Find(NAME_Actor) != std::string::npos && c != NAME_ActorFactory)
  {
    return new UActor(exp);
  }
  return new UObject(exp);
}