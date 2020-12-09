#include "ULight.h"

bool ULight::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(LightComponent, ULightComponent*);
  return false;
}

void ULight::PostLoad()
{
  LoadObject(LightComponent);
  Super::PostLoad();
}

bool ULightComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(Brightness);
  REGISTER_COL_PROP(LightColor);
  REGISTER_BOOL_PROP(CastShadows);
  REGISTER_BOOL_PROP(CastStaticShadows);
  REGISTER_BOOL_PROP(CastDynamicShadows);
  return false;
}

bool UPointLightComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(Radius);
  REGISTER_FLOAT_PROP(FalloffExponent);
  REGISTER_FLOAT_PROP(ShadowFalloffExponent);
  return false;
}

bool USpotLightComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(InnerConeAngle);
  REGISTER_FLOAT_PROP(OuterConeAngle);
  return false;
}
