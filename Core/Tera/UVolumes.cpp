#include "UVolumes.h"
#include "UModel.h"
#include "UActorComponent.h"

bool UBrush::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(Brush, UModel*);
  REGISTER_TOBJ_PROP(BrushComponent, UBrushComponent*);
  return false;
}
