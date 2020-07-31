#include "UComponent.h"
#include "FStream.h"
#include "UClass.h"

void UComponent::PreSerialize(FStream& s)
{
  s << TemplateOwnerClass;
  if (IsTemplate(RF_ClassDefaultObject))
  {
    s << TemplateName;
  }
}
