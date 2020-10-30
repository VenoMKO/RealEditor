#include "UActorComponent.h"
#include "Cast.h"

#include "FPackage.h"

bool UActorComponent::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, Translation))
  {
    TranslationProperty = property;
    property->GetVector(Translation);
    return true;
  }
  if (PROP_IS(property, Rotation))
  {
    RotationProperty = property;
    property->GetRotator(Rotation);
    return true;
  }
  if (PROP_IS(property, Scale))
  {
    ScaleProperty = property;
    Scale = property->Value->GetFloat();
    return true;
  }
  if (PROP_IS(property, Scale3D))
  {
    Scale3DProperty = property;
    property->GetVector(Scale3D);
    return true;
  }
  return false;
}

bool UPrimitiveComponent::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, ReplacementPrimitive))
  {
    ReplacementPrimitiveProperty = property;
    ReplacementPrimitive = Cast<UPrimitiveComponent>(GetPackage()->GetObject(property->Value->GetObjectIndex(), false));
    return true;
  }
  if (PROP_IS(property, CastShadow))
  {
    CastShadowProperty = property;
    CastShadow = property->BoolVal;
    return true;
  }
  return false;
}

void UPrimitiveComponent::PostLoad()
{
  Super::PostLoad();
  LoadObject(ReplacementPrimitive);
}