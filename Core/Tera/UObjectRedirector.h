#pragma once
#include "UObject.h"
#include "FPackage.h"

class UObjectRedirector : public UObject {
public:
  DECL_UOBJ(UObjectRedirector, UObject);

  UObject* GetObject() const
  {
    return Object;
  }

  UObject* GetAltObject() const
  {
    return AltObject;
  }

  PACKAGE_INDEX GetObjectRefIndex() const
  {
    return ObjectRefIndex;
  }
  
  void Serialize(FStream& s) override;

  friend void FPackage::ConvertObjectToRedirector(UObject*& source, UObject* targer);

private:
  DECL_UREF(UObject, Object);
  UObject* AltObject = nullptr;
};