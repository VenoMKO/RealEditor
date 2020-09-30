#include "UMaterial.h"
#include "UTexture.h"
#include "FPackage.h"
#include "Cast.h"

bool UMaterialInstanceConstant::RegisterProperty(FPropertyTag* property)
{
  if (property->Name == "TextureParameterValues")
  {
    if (property->Value->GetArray().empty())
    {
      return false;
    }
    TextureParameterValues = property->Value->GetArrayPtr();
    return true;
  }
  return false;
}

UTexture2D* UMaterialInstanceConstant::GetDiffuseTexture() const
{
  UTexture2D* result = GetTextureParameterValue("DiffuseMap");
  if (!result)
  {
    result = GetTextureParameterValue("PPC_HairDiffuseMap");
  }
  return result;
}

UTexture2D* UMaterialInstanceConstant::GetTextureParameterValue(const FString& name) const
{
  if (!TextureParameterValues)
  {
    return nullptr;
  }
  bool thisValue = false;
  for (FPropertyValue* container : *TextureParameterValues)
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
