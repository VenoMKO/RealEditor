#pragma once
#include "UObject.h"

class UObjectReferencer : public UObject {
public:
  DECL_UOBJ(UObjectReferencer, UObject);

  UPROP_CREATABLE_ARR_PTR(ReferencedObjects);

  bool RegisterProperty(FPropertyTag* property) override;

  void AddObject(UObject* obj);
  std::vector<UObject*> GetObject();
};