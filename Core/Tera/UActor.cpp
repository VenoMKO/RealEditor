#include "UActor.h"
#include "FPackage.h"

bool UActor::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, Components))
  {
    ComponentsProperty = property;
    auto& tarray = property->Value->GetArray();
    for (FPropertyValue* v : tarray)
    {
      Components.push_back((UActorComponent*)GetPackage()->GetObject(v->GetObjectIndex(), false));
    }
    return true;
  }
}

void UActor::PostLoad()
{
  for (UObject* obj : Components)
  {
    if (obj)
    {
      obj->Load();
    }
  }
  Super::PostLoad();
}
