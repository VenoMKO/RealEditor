#pragma once
#include "UObject.h"

class UObjectReferencer : public UObject {
public:
  DECL_UOBJ(UObjectReferencer, UObject);

  UPROP_CREATABLE_ARR_PTR(ReferencedObjects);

  bool RegisterProperty(FPropertyTag* property) override;

  void AddObject(UObject* obj);
  void RemoveExport(FObjectExport* exp);
  std::vector<UObject*> GetObject();
};