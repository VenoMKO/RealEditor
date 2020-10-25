#pragma once
#include "UComponent.h"

class UActorComponent : public UComponent {
public:
  DECL_UOBJ(UActorComponent, UComponent);
};

class UPrimitiveComponent : public UActorComponent {
public:
  DECL_UOBJ(UPrimitiveComponent, UActorComponent);
};