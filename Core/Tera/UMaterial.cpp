#include "UMaterial.h"
#include "UTexture.h"
#include "FPackage.h"
#include "Cast.h"

bool UMaterialInterface::RegisterProperty(FPropertyTag* property)
{
  if (PROP_IS(property, TextureParameterValues))
  {
    TextureParameterValues = property->Value->GetArray();
    TextureParameterValuesProperty = property;
    return true;
  }
  if (PROP_IS(property, BlendMode))
  {
    BlendMode = (EBlendMode)property->Value->GetByte();
    BlendModeProperty = property;
    return true;
  }
  if (PROP_IS(property, Parent))
  {
    Parent = property->Value->GetObjectIndex();
    ParentProperty = property;
    return true;
  }
  return false;
}

UTexture2D* UMaterialInterface::GetDiffuseTexture() const
{
  UTexture2D* result = GetTextureParameterValue("PCC_HairDiffuseMap");
  if (!result)
  {
    result = GetTextureParameterValue("DiffuseMap");
  }
  return result;
}

EBlendMode UMaterialInterface::GetBlendMode() const
{
  if (BlendModeProperty)
  {
    return BlendMode;
  }
  UMaterialInterface* parent = Cast<UMaterialInterface>(GetParent());
  while (parent)
  {
    if (parent->BlendModeProperty)
    {
      return parent->GetBlendMode();
    }
    parent = Cast<UMaterialInterface>(parent->GetParent());
  }
  return BlendMode;
}

UObject* UMaterialInterface::GetParent() const
{
  return ParentProperty ? GetPackage()->GetObject(Parent) : nullptr;
}

UTexture2D* UMaterialInterface::GetTextureParameterValue(const FString& name) const
{
  bool thisValue = false;
  for (FPropertyValue* container : TextureParameterValues)
  {
    std::vector<FPropertyValue*>& tmpArray = container->GetArray();
    for (FPropertyValue* subcontainer : tmpArray)
    {
      FPropertyTag* tmpTag = subcontainer->GetPropertyTagPtr();
      if (tmpTag->Name == "ParameterName" && tmpTag->Value->GetName() == name)
      {
        thisValue = true;
        continue;
      }
      else if (thisValue && tmpTag->Name == "ParameterValue")
      {
        PACKAGE_INDEX objIndex = tmpTag->Value->GetObjectIndex();
        thisValue = false;
        if (objIndex)
        {
          return Cast<UTexture2D>(GetPackage()->GetObject(objIndex));
        }
      }
    }
  }
  return nullptr;
}
