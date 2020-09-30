#pragma once
#include "UObject.h"

class UTexture2D;
class UMaterialInstanceConstant : public UObject {
public:
  DECL_UOBJ(UMaterialInstanceConstant, UObject);

  bool RegisterProperty(FPropertyTag* property) override;

  UTexture2D* GetDiffuseTexture() const;

protected:
  UTexture2D* GetTextureParameterValue(const FString& name) const;

protected:
  std::vector<FPropertyValue*>* TextureParameterValues = nullptr;
};

// UObjectFactory uses 'Component' keyword to detec UComponents but
// UMaterialExpressionComponentMask is not a component. Define it here as a UObject
// to fix incorrect class construction
class UMaterialExpressionComponentMask : public UObject {
public:
  DECL_UOBJ(UMaterialExpressionComponentMask, UObject);
};