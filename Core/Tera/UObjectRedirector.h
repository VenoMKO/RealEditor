#pragma once
#include "UObject.h"
#include "FPackage.h"

class UObjectRedirector : public UObject {
public:
  DECL_UOBJ(UObjectRedirector, UObject);

  UObject* GetObject(bool recursive = false) const
  {
    if (Object && recursive && Object->GetClassName() == UObjectRedirector::StaticClassName())
    {
      // This shouldn't happen ever.
      // Redirectors must not point to other redirectors.
      DBreak();
      if (UObject* tmp = ((UObjectRedirector*)Object)->GetObject(true))
      {
        return tmp;
      }
    }
    return Object;
  }

  PACKAGE_INDEX GetObjectRefIndex() const
  {
    return ObjectRefIndex;
  }
  
  void Serialize(FStream& s) override;

  friend void FPackage::ConvertObjectToRedirector(UObject*& source, UObject* targer);

private:
  DECL_UREF(UObject, Object);
};