#pragma once
#include "UObject.h"

class UObjectRedirector : public UObject {
  DECL_UOBJ(UObjectRedirector, UObject);

  UObject* GetObject() const
  {
    return Object;
  }

protected:
  void Serialize(FStream& s) override;

private:
  UObject* Object = nullptr;
};