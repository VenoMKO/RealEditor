#include "UComponent.h"
#include "FStream.h"
#include "UClass.h"

void UComponent::PreSerialize(FStream& s)
{
  SERIALIZE_UREF(s, TemplateOwnerClass);
  if (IsTemplate(RF_ClassDefaultObject))
  {
    s << TemplateName;
  }
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
