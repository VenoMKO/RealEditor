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

  DECL_UREF(UClass, TemplateOwnerClass);
  FName TemplateName;
};