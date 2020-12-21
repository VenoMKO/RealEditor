#pragma once
#include "UObject.h"
#include "UActor.h"

class UPrefab : public UObject {
public:
  DECL_UOBJ(UPrefab, UObject);

  UPROP_NOINIT(std::vector<UObject*>, PrefabArchetypes);

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};

class UPrefabInstance : public UActor {
public:
  DECL_UOBJ(UPrefabInstance, UActor);

  UPROP(UPrefab*, TemplatePrefab, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};