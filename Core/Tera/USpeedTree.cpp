#include "USpeedTree.h"
#include "FPackage.h"
#include "FStream.h"
#include "UMaterial.h"

#include "Utils/ALog.h"

#define SPT_UserDataSection 19000
#define SPT_UserDataBegin 19002
#define SPT_UserDataEnd 19001


bool USpeedTree::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bLegacySpeedTree);
  REGISTER_INT_PROP(RandomSeed);
  REGISTER_TOBJ_PROP(BillboardMaterial, UMaterialInterface*);
  REGISTER_TOBJ_PROP(LeafMaterial, UMaterialInterface*);
  REGISTER_TOBJ_PROP(LeafMeshMaterial, UMaterialInterface*);
  REGISTER_TOBJ_PROP(LeafCardMaterial, UMaterialInterface*);
  REGISTER_TOBJ_PROP(FrondMaterial, UMaterialInterface*);
  REGISTER_TOBJ_PROP(BranchMaterial, UMaterialInterface*);
  REGISTER_TOBJ_PROP(Branch1Material, UMaterialInterface*);
  REGISTER_TOBJ_PROP(Branch2Material, UMaterialInterface*);
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
  if (!SpeedTreeDataSize)
  {
    return false;
  }
  FString branch = BranchMaterial ? BranchMaterial->GetObjectNameString() : FString();
  FString frond = FrondMaterial ? FrondMaterial->GetObjectNameString() : FString();
  FString leaf = LeafMaterial ? LeafMaterial->GetObjectNameString() : FString();

  if (embedMaterialInfo && (branch.Size() || frond.Size() || leaf.Size()))
  {
    FString userData;
    userData += '$';
    if (branch.Size())
    {
      userData += 'b';
      userData += (uint8)branch.UTF8().size();
      userData += branch.UTF8();
    }
    if (frond.Size())
    {
      userData += 'f';
      userData += (uint8)frond.UTF8().size();
      userData += frond.UTF8();
    }
    if (leaf.Size())
    {
      userData += 'l';
      userData += (uint8)leaf.UTF8().size();
      userData += leaf.UTF8();
    }
    userData += '\0';

    MWriteStream s(SpeedTreeData, SpeedTreeDataSize);
    s.SetPosition(SpeedTreeDataSize);
    uint32 tag;
    tag = SPT_UserDataSection;
    s << tag;
    tag = SPT_UserDataBegin;
    s << tag;
    s << userData;
    tag = SPT_UserDataEnd;
    s << tag;
    *outputSize = s.GetPosition();
    *output = malloc(*outputSize);
    memcpy(*output, s.GetAllocation(), *outputSize);
  }
  else
  {
    *output = malloc(SpeedTreeDataSize);
    memcpy(*output, SpeedTreeData, SpeedTreeDataSize);
  }
  return true;
}

void USpeedTree::SetSptData(void* data, FILE_OFFSET size)
{
  if (!SpeedTreeData)
  {
    SpeedTreeData = malloc(size);
    SpeedTreeDataSize = size;
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

bool USpeedTreeComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(SpeedTree, USpeedTree*);
  REGISTER_TOBJ_PROP(Branch1Material, UMaterialInterface*);
  REGISTER_TOBJ_PROP(Branch2Material, UMaterialInterface*);
  REGISTER_TOBJ_PROP(FrondMaterial, UMaterialInterface*);
  REGISTER_TOBJ_PROP(LeafMaterial, UMaterialInterface*);
  REGISTER_TOBJ_PROP(LeafCardMaterial, UMaterialInterface*);
  REGISTER_TOBJ_PROP(LeafMeshMaterial, UMaterialInterface*);
  REGISTER_TOBJ_PROP(BillboardMaterial, UMaterialInterface*);
  return false;
}

void USpeedTreeComponent::PostLoad()
{
  LoadObject(SpeedTree);
  LoadObject(LeafMaterial);
  LoadObject(Branch1Material);
  LoadObject(Branch2Material);
  LoadObject(FrondMaterial);
  LoadObject(LeafCardMaterial);
  LoadObject(LeafMeshMaterial);
  LoadObject(BillboardMaterial);
  Super::PostLoad();
}
