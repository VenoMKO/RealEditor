#pragma once
#include "UObject.h"

class UObjectRedirector : public UObject {
  DECL_UOBJ(UObjectRedirector, UObject);

protected:
  void Serialize(FStream& s) override;

private:
  UObject* Object = nullptr;
};