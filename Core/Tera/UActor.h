#pragma once
#include "UObject.h"
#include "UActorComponent.h"

class UActor : public UObject {
public:
  DECL_UOBJ(UActor, UObject);

  bool RegisterProperty(FPropertyTag* property) override;

  UPROP(std::vector<UActorComponent*>, Components, {});
  UPROP(FVector, Location, {});
  UPROP(FRotator, Rotation, {});
  UPROP(FVector, DrawScale3D, FVector(1, 1, 1));
  UPROP(float, DrawScale, 1.);

  void PostLoad() override;
};

class UStaticMeshComponent;
class UStaticMeshActor : public UActor {
public:
  DECL_UOBJ(UStaticMeshActor, UActor);

  bool RegisterProperty(FPropertyTag* property) override;

  UPROP(UStaticMeshComponent*, StaticMeshComponent, nullptr);

  void PostLoad() override;
};