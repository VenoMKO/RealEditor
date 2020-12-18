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

class UPointLightMovable : public UPointLight {
public:
  DECL_UOBJ(UPointLightMovable, UPointLight);
};

class UPointLightToggleable : public UPointLight {
public:
  DECL_UOBJ(UPointLightToggleable, UPointLight);
};

class USpotLight : public ULight {
public:
  DECL_UOBJ(USpotLight, ULight);
};

class USpotLightMovable : public USpotLight {
public:
  DECL_UOBJ(USpotLightMovable, USpotLight);
};

class USpotLightToggleable : public USpotLight {
public:
  DECL_UOBJ(USpotLightToggleable, USpotLight);
};

class UDirectionalLight : public ULight {
public:
  DECL_UOBJ(UDirectionalLight, ULight);
};

class UDirectionalLightToggleable : public UDirectionalLight {
public:
  DECL_UOBJ(UDirectionalLightToggleable, UDirectionalLight);
};

class USkyLight : public ULight {
public:
  DECL_UOBJ(USkyLight, ULight);
};

class USkyLightToggleable : public USkyLight {
public:
  DECL_UOBJ(USkyLightToggleable, USkyLight);
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
  UPROP(bool, bEnabled, true);

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

  USpotLightComponent(FObjectExport* exp);

  UPROP(float, InnerConeAngle, 0.);
  UPROP(float, OuterConeAngle, 44.);

  bool RegisterProperty(FPropertyTag* property) override;
};

class UDirectionalLightComponent : public ULightComponent {
public:
  DECL_UOBJ(UDirectionalLightComponent, ULightComponent);

  UDirectionalLightComponent(FObjectExport* exp);
};

class UDominantDirectionalLightComponent : public ULightComponent {
  DECL_UOBJ(UDominantDirectionalLightComponent, ULightComponent);

  void Serialize(FStream& s) override;
protected:
  std::vector<uint16> DominantLightShadowMap;
};

class UDominantSpotLightComponent : public ULightComponent {
  DECL_UOBJ(UDominantSpotLightComponent, ULightComponent);

  void Serialize(FStream& s) override;
protected:
  std::vector<uint16> DominantLightShadowMap;
};

class USkyLightComponent : public ULightComponent {
public:
  DECL_UOBJ(USkyLightComponent, ULightComponent);

  USkyLightComponent(FObjectExport* exp);

  UPROP(float, LowerBrightness, 0);
  UPROP(FColor, LowerColor, FColor(255, 255, 255, 255));

  bool RegisterProperty(FPropertyTag* property) override;
};