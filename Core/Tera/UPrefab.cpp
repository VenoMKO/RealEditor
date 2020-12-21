#include "UPrefab.h"
#include "FPackage.h"

bool UPrefab::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, PrefabArchetypes))
  {
    PrefabArchetypes.clear();
    for (FPropertyValue* objValue : property->GetArray())
    {
      PrefabArchetypes.push_back(GetPackage()->GetObject(objValue->GetObjectIndex(), false));
    }
    return true;
  }
  return false;
}

void UPrefab::PostLoad()
{
  Super::PostLoad();
  for (UObject* arch : PrefabArchetypes)
  {
    LoadObject(arch);
  }
}

bool UPrefabInstance::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(TemplatePrefab, UPrefab*);
  return false;
}

void UPrefabInstance::PostLoad()
{
  Super::PostLoad();
  LoadObject(TemplatePrefab);
}
