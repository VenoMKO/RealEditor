#pragma once
#include "UActor.h"

class UBrush : public UActor {
public:
  DECL_UOBJ(UBrush, UActor);

  UPROP(class UModel*, Brush, nullptr);
  UPROP(class UBrushComponent*, BrushComponent, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;
};

class UVolume : public UBrush {
public:
  DECL_UOBJ(UVolume, UBrush);
};

class UBlockingVolume : public UVolume {
public:
  DECL_UOBJ(UBlockingVolume, UVolume);

};