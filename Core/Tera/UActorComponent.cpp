#include "UActorComponent.h"
#include "Cast.h"

#include "FPackage.h"

bool UActorComponent::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  SUPER_REGISTER_PROP();
  REGISTER_VEC_PROP(Translation);
  REGISTER_VEC_PROP(Scale3D);
  REGISTER_ROT_PROP(Rotation);
  REGISTER_FLOAT_PROP(Scale);
  return false;
}

bool UPrimitiveComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(ReplacementPrimitive, UPrimitiveComponent*);
  REGISTER_BOOL_PROP(CastShadow);
  REGISTER_BOOL_PROP(bCastDynamicShadow);
  REGISTER_BOOL_PROP(bCastStaticShadow);
  REGISTER_FLOAT_PROP(MinDrawDistance);
  REGISTER_FLOAT_PROP(MaxDrawDistance);
  REGISTER_FLOAT_PROP(CachedMaxDrawDistance);
  REGISTER_BOOL_PROP(bAcceptsLights);
  REGISTER_BOOL_PROP(HiddenGame);
  REGISTER_BOOL_PROP(HiddenEditor);
  return false;
}

void UPrimitiveComponent::PostLoad()
{
  Super::PostLoad();
  LoadObject(ReplacementPrimitive);
}

bool UMeshComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, Materials))
  {
    for (FPropertyValue* v : property->GetArray())
    {
      Materials.push_back(GetPackage()->GetObject(v->GetObjectIndex(), false));
    }
    return true;
  }
  return false;
}

void UMeshComponent::PostLoad()
{
  Super::PostLoad();
  for (UObject* mat : Materials)
  {
    LoadObject(mat);
  }
}

bool UHeightFogComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bEnabled);
  REGISTER_FLOAT_PROP(Density);
  REGISTER_COL_PROP(LightColor);
  REGISTER_FLOAT_PROP(ExtinctionDistance);
  return false;
}

bool UBrushComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(Brush, UModel*);
  return false;
}

void UBrushComponent::PostLoad()
{
  Super::PostLoad();
  for (FPropertyTag* tag : Properties)
  {
    if (tag->Name == "BrushAggGeom")
    {
      LoadAggGeom(tag->GetArray(), GetPackage());
      break;
    }
  }
}