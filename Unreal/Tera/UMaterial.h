#pragma once
#include "UObject.h"

// UObjectFactory uses 'Component' keyword to detec UComponents but
// UMaterialExpressionComponentMask is not a component. Define it here as a UObject
// to fix incorrect class construction
class UMaterialExpressionComponentMask : public UObject {
public:
  DECL_UOBJ(UMaterialExpressionComponentMask, UObject);
};