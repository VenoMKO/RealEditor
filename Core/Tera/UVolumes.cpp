#include "UVolumes.h"
#include "UModel.h"
#include "UActorComponent.h"
#include "USoundNode.h"
#include "UClass.h"

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

bool UReverbVolume::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, Settings))
  {
    SettingsProperty = property;
    return true;
  }
  return false;
}

void UReverbVolume::PostLoad()
{
  if (SettingsProperty)
  {
    for (FPropertyValue* tmp : SettingsProperty->GetArray())
    {
      if (tmp->Type != FPropertyValue::VID::Property)
      {
        continue;
      }
      FString name = tmp->GetPropertyTag().Name.String();
      if (name == "ReverbType")
      {
        Settings.ReverbType = (ReverbPreset)tmp->GetPropertyTag().GetByte();
      }
      else if (name == "Volume")
      {
        Settings.Volume = tmp->GetPropertyTag().GetFloat();
      }
      else if (name == "FadeTime")
      {
        Settings.FadeTime = tmp->GetPropertyTag().GetFloat();
      }
    }
  }
  Super::PostLoad();
}
