#pragma once
#include "UComponent.h"

class UActorComponent : public UComponent {
public:
  DECL_UOBJ(UActorComponent, UComponent);

  bool RegisterProperty(FPropertyTag* property) override;

  UPROP(FVector, Translation, {});
  UPROP(FRotator, Rotation, {});
  UPROP(FVector, Scale3D, FVector(1, 1, 1));
  UPROP(float, Scale, 1.);
};

class UPrimitiveComponent : public UActorComponent {
public:
  DECL_UOBJ(UPrimitiveComponent, UActorComponent);

  bool RegisterProperty(FPropertyTag* property) override;

  UPROP(UPrimitiveComponent*, ReplacementPrimitive, nullptr);
  UPROP(bool, CastShadow, true);

  void PostLoad() override;
};

class UMeshComponent : public UPrimitiveComponent {
public:
  DECL_UOBJ(UMeshComponent, UPrimitiveComponent);
};