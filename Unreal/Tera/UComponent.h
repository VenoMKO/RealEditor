#pragma once
#include "UObject.h"
#include "FName.h"

class UComponent : public UObject {
public:
  DECL_UOBJ(UComponent, UObject);

  void PreSerialize(FStream& s);

  bool IsComponent() const override
  {
    return true;
  }

  UClass* TemplateOwnerClass = nullptr;
  FName TemplateName;
};

// Need this component because it has an unusual serializtion
// that breaks UComponent::Serialize if not implemented
class UDominantDirectionalLightComponent : public UComponent {
  DECL_UOBJ(UDominantDirectionalLightComponent, UComponent);

  void Serialize(FStream& s) override;
protected:
  std::vector<uint16> DominantLightShadowMap;
};

// Need this component because it has an unusual serializtion
// that breaks UComponent::Serialize if not implemented
class UDominantSpotLightComponent : public UComponent {
  DECL_UOBJ(UDominantSpotLightComponent, UComponent);

  void Serialize(FStream& s) override;
protected:
  std::vector<uint16> DominantLightShadowMap;
};