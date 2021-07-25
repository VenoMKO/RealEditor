#pragma once
#include "UComponent.h"
#include "UPhysAsset.h"

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
  UPROP(bool, bCastDynamicShadow, true);
  UPROP(bool, bCastStaticShadow, true);
  UPROP(float, MinDrawDistance, 0);
  UPROP(float, MaxDrawDistance, 0);
  UPROP(float, CachedMaxDrawDistance, 0);
  UPROP(bool, bAcceptsLights, true);
  UPROP(bool, HiddenGame, false);
  UPROP(bool, HiddenEditor, false);

  void PostLoad() override;
};

class UMeshComponent : public UPrimitiveComponent {
public:
  DECL_UOBJ(UMeshComponent, UPrimitiveComponent);

  UPROP_NOINIT(std::vector<UObject*>, Materials);

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};

class UHeightFogComponent : public UActorComponent {
public:
  DECL_UOBJ(UHeightFogComponent, UActorComponent);

  UPROP(bool, bEnabled, true);
  UPROP(float, Density, 0.00005f);
  UPROP(FColor, LightColor, FColor(255, 255, 255, 255));
  UPROP(float, StartDistance, 0.f);
  UPROP(float, ExtinctionDistance, 100000000.f);

  bool RegisterProperty(FPropertyTag* property) override;
};

class UBrushComponent 
  : public UPrimitiveComponent
  , public AggGeomOwner {
public:
  DECL_UOBJ(UBrushComponent, UPrimitiveComponent);

  UPROP(class UModel*, Brush, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;

  void PostLoad() override;
};