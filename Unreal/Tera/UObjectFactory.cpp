#include "UObject.h"
#include "FObjectResource.h"
#include "FName.h"

#include "UClass.h"
#include "UProperty.h"

// Components
#include "UComponent.h"

// Objects
#include "UObjectRedirector.h"
#include "UMaterial.h"

UObject* UObject::Object(FObjectExport* exp)
{
  if (exp->Object)
  {
    return exp->Object;
  }
  UObject* result = nullptr;
  const FString c = exp->GetClassName();
  if (c == UClass::StaticClassName())
  {
    result = new UClass(exp);
  }
  else if (c == UField::StaticClassName())
  {
    result = new UField(exp);
  }
  else if (c == UStruct::StaticClassName())
  {
    result = new UStruct(exp);
  }
  else if (c == UScriptStruct::StaticClassName())
  {
    result = new UScriptStruct(exp);
  }
  else if (c == UState::StaticClassName())
  {
    result = new UState(exp);
  }
  else if (c == UEnum::StaticClassName())
  {
    result = new UEnum(exp);
  }
  else if (c == UConst::StaticClassName())
  {
    result = new UConst(exp);
  }
  else if (c == UFunction::StaticClassName())
  {
    result = new UFunction(exp);
  }
  else if (c == UTextBuffer::StaticClassName())
  {
    result = new UTextBuffer(exp);
  }
  else if (c == UIntProperty::StaticClassName())
  {
    result = new UIntProperty(exp);
  }
  else if (c == UBoolProperty::StaticClassName())
  {
    result = new UBoolProperty(exp);
  }
  else if (c == UByteProperty::StaticClassName())
  {
    result = new UByteProperty(exp);
  }
  else if (c == UFloatProperty::StaticClassName())
  {
    result = new UFloatProperty(exp);
  }
  else if (c == UObjectProperty::StaticClassName())
  {
    result = new UObjectProperty(exp);
  }
  else if (c == UClassProperty::StaticClassName())
  {
    result = new UClassProperty(exp);
  }
  else if (c == UComponentProperty::StaticClassName())
  {
    result = new UComponentProperty(exp);
  }
  else if (c == UNameProperty::StaticClassName())
  {
    result = new UNameProperty(exp);
  }
  else if (c == UStrProperty::StaticClassName())
  {
    result = new UStrProperty(exp);
  }
  else if (c == UStructProperty::StaticClassName())
  {
    result = new UStructProperty(exp);
  }
  else if (c == UArrayProperty::StaticClassName())
  {
    result = new UArrayProperty(exp);
  }
  else if (c == UMapProperty::StaticClassName())
  {
    result = new UMapProperty(exp);
  }
  else if (c == UInterfaceProperty::StaticClassName())
  {
    result = new UInterfaceProperty(exp);
  }
  else if (c == UDelegateProperty::StaticClassName())
  {
    result = new UDelegateProperty(exp);
  }
  else if (c == UObjectRedirector::StaticClassName())
  {
    result = new UObjectRedirector(exp);
  }
  else if (c == UMaterialExpressionComponentMask::StaticClassName())
  {
    result = new UMaterialExpressionComponentMask(exp);
  }
  else if (c == UComponent::StaticClassName())
  {
    result = new UComponent(exp);
  }
  else if (c == UDominantDirectionalLightComponent::StaticClassName())
  {
    result = new UDominantDirectionalLightComponent(exp);
  }
  else if (c == UDominantSpotLightComponent::StaticClassName())
  {
    result = new UDominantSpotLightComponent(exp);
  }
  else
  {
    // Fallback for unimplemented components. *Component => UComponent
    if ((c.Find(UComponent::StaticClassName()) != std::string::npos) ||
        (c.Find("Distribution") != std::string::npos))
    {
      result = new UComponent(exp);
    }
    else
    {
      result = new UObject(exp);
    }
  }
  exp->Object = result;
  return result;
}