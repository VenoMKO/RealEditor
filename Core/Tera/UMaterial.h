#pragma once
#include "UObject.h"

class UTexture2D;

class UMaterialInterface : public UObject {
public:
  DECL_UOBJ(UMaterialInterface, UObject);

  bool RegisterProperty(FPropertyTag* property) override;

  UTexture2D* GetTextureParameterValue(const FString& name) const;
  UTexture2D* GetDiffuseTexture() const;
  EBlendMode GetBlendMode() const;
  UObject* GetParent() const;

protected:
  UPROP(std::vector<FPropertyValue*>, TextureParameterValues, {});
  UPROP(EBlendMode, BlendMode, EBlendMode::BLEND_Opaque);
  UPROP(PACKAGE_INDEX, Parent, INDEX_NONE);
};

class UMaterial : public UMaterialInterface {
public:
  DECL_UOBJ(UMaterial, UMaterialInterface);
  // TODO: Serialize static permutation data
  // TODO: Render expressions graph
};

class UMaterialInstance : public UMaterialInterface {
public:
  DECL_UOBJ(UMaterialInstance, UMaterialInterface);
};

class UMaterialInstanceConstant : public UMaterialInstance {
public:
  DECL_UOBJ(UMaterialInstanceConstant, UMaterialInstance);
};

// UObjectFactory uses 'Component' keyword to detec UComponents but
// UMaterialExpressionComponentMask is not a component. Define it here as a UObject
// to fix incorrect class construction
class UMaterialExpressionComponentMask : public UObject {
public:
  DECL_UOBJ(UMaterialExpressionComponentMask, UObject);
};