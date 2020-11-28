#include "USpeedTree.h"
#include "FPackage.h"

#include "ALog.h"

bool USpeedTree::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, bLegacySpeedTree))
  {
    bLegacySpeedTreeProperty = property;
    bLegacySpeedTree = property->BoolVal;
    return true;
  }
  if (PROP_IS(property, RandomSeed))
  {
    RandomSeedProperty = property;
    RandomSeed = property->Value->GetInt();
    return true;
  }
  if (PROP_IS(property, BillboardMaterial))
  {
    BillboardMaterialProperty = property;
    BillboardMaterial = GetPackage()->GetObject(property->Value->GetObjectIndex(), false);
    return true;
  }
  if (PROP_IS(property, LeafMaterial))
  {
    LeafMaterialProperty = property;
    LeafMaterial = GetPackage()->GetObject(property->Value->GetObjectIndex(), false);
    return true;
  }
  if (PROP_IS(property, LeafMeshMaterial))
  {
    LeafMeshMaterialProperty = property;
    LeafMeshMaterial = GetPackage()->GetObject(property->Value->GetObjectIndex(), false);
    return true;
  }
  if (PROP_IS(property, LeafCardMaterial))
  {
    LeafCardMaterialProperty = property;
    LeafCardMaterial = GetPackage()->GetObject(property->Value->GetObjectIndex(), false);
    return true;
  }
  if (PROP_IS(property, FrondMaterial))
  {
    FrondMaterialProperty = property;
    FrondMaterial = GetPackage()->GetObject(property->Value->GetObjectIndex(), false);
    return true;
  }
  if (PROP_IS(property, BranchMaterial))
  {
    BranchMaterialProperty = property;
    BranchMaterial = GetPackage()->GetObject(property->Value->GetObjectIndex(), false);
    return true;
  }
  if (PROP_IS(property, Branch1Material))
  {
    Branch1MaterialProperty = property;
    Branch1Material = GetPackage()->GetObject(property->Value->GetObjectIndex(), false);
    return true;
  }
  if (PROP_IS(property, Branch2Material))
  {
    Branch2MaterialProperty = property;
    Branch2Material = GetPackage()->GetObject(property->Value->GetObjectIndex(), false);
    return true;
  }
  return false;
}

void USpeedTree::PostLoad()
{
  Super::PostLoad();
  LoadObject(BillboardMaterial);
  LoadObject(LeafMaterial);
  LoadObject(LeafMeshMaterial);
  LoadObject(LeafCardMaterial);
  LoadObject(FrondMaterial);
  LoadObject(BranchMaterial);
  LoadObject(Branch1Material);
  LoadObject(Branch2Material);
}

void USpeedTree::Serialize(FStream& s)
{
  Super::Serialize(s);
  DBreakIf(!bLegacySpeedTree || bLegacySpeedTreeProperty);
  s << SpeedTreeDataSize;
  if (s.IsReading() && SpeedTreeDataSize)
  {
    SpeedTreeData = malloc(SpeedTreeDataSize);
  }
  s.SerializeBytes(SpeedTreeData, SpeedTreeDataSize);
}

bool USpeedTree::GetSptData(void** output, FILE_OFFSET* outputSize, bool embedMaterialInfo)
{
  if (!output || !bLegacySpeedTree)
  {
    return false;
  }
  *output = nullptr;
  *outputSize = SpeedTreeDataSize;
  if (SpeedTreeDataSize)
  {
    *output = malloc(SpeedTreeDataSize);
    memcpy(*output, SpeedTreeData, SpeedTreeDataSize);
    // TODO: embed materials
    return true;
  }
  return false;
}

void USpeedTree::SetSptData(void* data, FILE_OFFSET size)
{
  if (!SpeedTreeData)
  {
    SpeedTreeData = malloc(size);
  }
  else if (SpeedTreeDataSize != size)
  {
    if (void* tmp = realloc(SpeedTreeData, size))
    {
      SpeedTreeData = tmp;
      SpeedTreeDataSize = size;
    }
    else
    {
      LogE("Failed to allocate SPT data!");
      return;
    }
  }

  memcpy(SpeedTreeData, data, size);
  MarkDirty();
}
