#pragma once
#include "UObject.h"
#include "UActorComponent.h"

class USpeedTree : public UObject {
public:
  DECL_UOBJ(USpeedTree, UObject);
  UPROP(bool, bLegacySpeedTree, true); // In vanilla UE3 this should be false by default
  UPROP(int32, RandomSeed, 1);
  UPROP(UObject*, BillboardMaterial, nullptr);
  UPROP(UObject*, LeafMaterial, nullptr);
  UPROP(UObject*, LeafMeshMaterial, nullptr);
  UPROP(UObject*, LeafCardMaterial, nullptr);
  UPROP(UObject*, FrondMaterial, nullptr);
  UPROP(UObject*, BranchMaterial, nullptr);
  UPROP(UObject*, Branch1Material, nullptr);
  UPROP(UObject*, Branch2Material, nullptr);

  ~USpeedTree() override
  {
    free(SpeedTreeData);
  }

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
  void Serialize(FStream& s) override;
  bool GetSptData(void** output, FILE_OFFSET* outputSize, bool embedMaterialInfo);
  void SetSptData(void* data, FILE_OFFSET size);

protected:
  FILE_OFFSET SpeedTreeDataSize = 0;
  void* SpeedTreeData = nullptr;
};

class UMaterialInterface;
class USpeedTreeComponent : public UMeshComponent {
public:
  DECL_UOBJ(USpeedTreeComponent, UMeshComponent);

  UPROP(USpeedTree*, SpeedTree, nullptr);
  UPROP(UMaterialInterface*, BranchMaterial, nullptr);
  UPROP(UMaterialInterface*, Branch1Material, nullptr);
  UPROP(UMaterialInterface*, Branch2Material, nullptr);
  UPROP(UMaterialInterface*, FrondMaterial, nullptr);
  UPROP(UMaterialInterface*, LeafMaterial, nullptr);
  UPROP(UMaterialInterface*, LeafCardMaterial, nullptr);
  UPROP(UMaterialInterface*, LeafMeshMaterial, nullptr);
  UPROP(UMaterialInterface*, BillboardMaterial, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};