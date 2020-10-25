#pragma once
#include "UObject.h"
#include "UActorComponent.h"

class UActor : public UObject {
public:
  DECL_UOBJ(UActor, UObject);

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;

  UPROP(std::vector<UActorComponent*>, Components, {});
};