#pragma once
#include "UObject.h"

class UObjectRedirector : public UObject {
  DECL_UOBJ(UObjectRedirector, UObject);

  UObject* GetObject() const
  {
    return Object;
  }

  PACKAGE_INDEX GetObjectRefIndex() const
  {
    return ObjectRefIndex;
  }
  
  void Serialize(FStream& s) override;

private:
  DECL_UREF(UObject, Object);
};