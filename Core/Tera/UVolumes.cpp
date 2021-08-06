#include "UVolumes.h"
#include "UModel.h"
#include "UActorComponent.h"
#include "USoundNode.h"

bool UBrush::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(Brush, UModel*);
  REGISTER_TOBJ_PROP(BrushComponent, UBrushComponent*);
  return false;
}

bool US1MusicVolume::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, MusicList))
  {
    MusicListProperty = property;
    for (FPropertyValue* val : MusicListProperty->GetArray())
    {
      if (USoundCue* cue = Cast<USoundCue>(val->GetObjectValuePtr(false)))
      {
        cue->Load();
        MusicList.emplace_back(cue);
      }
    }
    return true;
  }
  return false;
}

std::vector<USoundNodeWave*> US1MusicVolume::GetAllWaves() const
{
  std::vector<USoundNodeWave*> result;
  for (USoundCue* cue : MusicList)
  {
    cue->GetWaves(result);
  }
  return result;
}
