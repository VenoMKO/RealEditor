#pragma once
#include "UObject.h"
#include "UActor.h"

class ULightComponent;
class ULight : public UActor {
public:
  DECL_UOBJ(ULight, UActor);
  
  UPROP(bool, bEnabled, true);
  UPROP(ULightComponent*, LightComponent, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};

class UPointLight : public ULight {
public:
  DECL_UOBJ(UPointLight, ULight);
};

class USpotLight : public ULight {
public:
  DECL_UOBJ(USpotLight, ULight);
};

class ULightComponent : public UActorComponent {
public:
  DECL_UOBJ(ULightComponent, UActorComponent);

  UPROP_NOINIT(FGuid, LightGuid);
  UPROP(float, Brightness, 1.);
  UPROP(FColor, LightColor, FColor(255, 255, 255, 255));
  UPROP(bool, CastShadows, true);
  UPROP(bool, CastStaticShadows, true);
  UPROP(bool, CastDynamicShadows, false);

  bool RegisterProperty(FPropertyTag* property) override;
};

class UPointLightComponent : public ULightComponent {
public:
  DECL_UOBJ(UPointLightComponent, ULightComponent);

  UPROP(float, Radius, 1024.);
  UPROP(float, FalloffExponent, 2);
  UPROP(float, ShadowFalloffExponent, 2);

  bool RegisterProperty(FPropertyTag* property) override;
};

class USpotLightComponent : public UPointLightComponent {
public:
  DECL_UOBJ(USpotLightComponent, UPointLightComponent);

  UPROP(float, InnerConeAngle, 0.);
  UPROP(float, OuterConeAngle, 44.);

  bool RegisterProperty(FPropertyTag* property) override;
};