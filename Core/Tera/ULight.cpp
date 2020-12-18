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
  REGISTER_BOOL_PROP(bEnabled);
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

USpotLightComponent::USpotLightComponent(FObjectExport* exp)
  : UPointLightComponent(exp)
{
  CastDynamicShadows = true;
}

bool USpotLightComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(InnerConeAngle);
  REGISTER_FLOAT_PROP(OuterConeAngle);
  return false;
}

UDirectionalLightComponent::UDirectionalLightComponent(FObjectExport* exp)
  : ULightComponent(exp)
{
  CastDynamicShadows = true;
}

void UDominantDirectionalLightComponent::Serialize(FStream& s)
{
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << DominantLightShadowMap;
  }
  Super::Serialize(s);
}

void UDominantSpotLightComponent::Serialize(FStream& s)
{
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << DominantLightShadowMap;
  }
  Super::Serialize(s);
}

USkyLightComponent::USkyLightComponent(FObjectExport* exp)
  : ULightComponent(exp)
{
  CastShadows = false;
}

bool USkyLightComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(LowerBrightness);
  REGISTER_COL_PROP(LowerColor);
  return false;
}
