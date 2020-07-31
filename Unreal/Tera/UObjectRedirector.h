#pragma once
#include "UObject.h"

class UObjectRedirector : public UObject {
  DECL_UOBJ(UObjectRedirector, UObject);

protected:
  virtual void Serialize(FStream& s) override;
  virtual void PostLoad() override;

private:
  UObject* Object = nullptr;
};