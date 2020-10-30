#include "UActor.h"
#include "FPackage.h"

#include "UClass.h"
#include "FPropertyTag.h"
#include "Cast.h"

#include "UActorComponent.h"
#include "UStaticMesh.h"

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
  if (PROP_IS(property, Location))
  {
    LocationProperty = property;
    property->GetVector(Location);
    return true;
  }
  if (PROP_IS(property, Rotation))
  {
    RotationProperty = property;
    property->GetRotator(Rotation);
    return true;
  }
  if (PROP_IS(property, DrawScale3D))
  {
    DrawScale3DProperty = property;
    property->GetVector(DrawScale3D);
    return true;
  }
  if (PROP_IS(property, DrawScale))
  {
    DrawScaleProperty = property;
    DrawScale = property->Value->GetFloat();
    return true;
  }
  return false;
}

void UActor::PostLoad()
{
  Super::PostLoad();
  for (UObject* obj : Components)
  {
    LoadObject(obj);
  }
}

bool UStaticMeshActor::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, StaticMeshComponent))
  {
    StaticMeshComponentProperty = property;
    StaticMeshComponent = Cast<UStaticMeshComponent>(GetPackage()->GetObject(property->Value->GetObjectIndex(), false));
    return true;
  }
  return false;
}

void UStaticMeshActor::PostLoad()
{
  Super::PostLoad();
  LoadObject(StaticMeshComponent);
}
