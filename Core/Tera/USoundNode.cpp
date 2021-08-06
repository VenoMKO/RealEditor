#include "USoundNode.h"
#include "UComponent.h"
#include "FPackage.h"
#include "UClass.h"
#include "Cast.h"

bool USoundNodeWave::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(Duration);
  REGISTER_INT_PROP(NumChannels);
  REGISTER_INT_PROP(SampleRate);
  return false;
}

void USoundNodeWave::Serialize(FStream& s)
{
  Super::Serialize(s);
  EditorData.Serialize(s, this);
  PCData.Serialize(s, this);
  XBoxData.Serialize(s, this);
  PS3Data.Serialize(s, this);
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    UnkData1.Serialize(s, this);
    UnkData2.Serialize(s, this);
    UnkData3.Serialize(s, this);
    UnkData4.Serialize(s, this);
    UnkData5.Serialize(s, this);
  }
}

void USoundNodeWave::PostLoad()
{
  if ((ResourceSize = PCData.GetBulkDataSize()))
  {
    PCData.GetCopy(&ResourceData);
  }
}

void USoundNodeWave::GetWaves(std::vector<class USoundNodeWave*>& waves)
{
  waves.emplace_back(this);
}

bool USoundCue::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_NAME_PROP(SoundGroup);
  REGISTER_TOBJ_PROP(FirstNode, USoundNode*);
  REGISTER_FLOAT_PROP(Duration);
  REGISTER_FLOAT_PROP(VolumeMultiplier);
  REGISTER_FLOAT_PROP(PitchMultiplier);
  REGISTER_FLOAT_PROP(MaxAudibleDistance);
  return false;
}

void USoundCue::GetWaves(std::vector<USoundNodeWave*>& waves)
{
  if (FirstNode)
  {
    FirstNode->GetWaves(waves);
  }
  Super::GetWaves(waves);
}

bool USoundNode::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, ChildNodes))
  {
    ChildNodesProperty = property;
    for (FPropertyValue* val : ChildNodesProperty->GetArray())
    {
      if (USoundNode* node = Cast<USoundNode>(val->GetObjectValuePtr(false)))
      {
        node->Load();
        ChildNodes.emplace_back(node);
      }
    }
    return true;
  }
  return false;
}

void USoundNode::GetWaves(std::vector<USoundNodeWave*>& waves)
{
  for (USoundNode* node : ChildNodes)
  {
    if (node)
    {
      node->GetWaves(waves);
    }
  }
}

bool USoundNodeAmbient::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(Wave, USoundNodeWave*);
  REGISTER_BOOL_PROP(bSpatialize);
  REGISTER_BOOL_PROP(bAttenuate);
  REGISTER_BOOL_PROP(bAttenuateWithLowPassFilter);
  REGISTER_BOOL_PROP(bAttenuateWithLPF);
  REGISTER_FLOAT_PROP(RadiusMin);
  REGISTER_FLOAT_PROP(RadiusMax);
  REGISTER_FLOAT_PROP(LPFRadiusMin);
  REGISTER_FLOAT_PROP(LPFRadiusMax);
  if (PROP_IS(property, DistanceModel))
  {
    DistanceModel = (SoundDistanceModel)property->GetByte();
    DistanceModelProperty = property;
    return true;
  }
  if (PROP_IS(property, MinRadius))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            MinRadius = (min + max) / 2.f;
            MinRadiusProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
    return false;
  }
  if (PROP_IS(property, MaxRadius))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            MaxRadius = (min + max) / 2.f;
            MaxRadiusProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
    return false;
  }
  if (PROP_IS(property, LPFMinRadius))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            LPFMinRadius = (min + max) / 2.f;
            LPFMinRadiusProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
    return false;
  }
  if (PROP_IS(property, LPFMaxRadius))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            LPFMaxRadius = (min + max) / 2.f;
            LPFMaxRadiusProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
    return false;
  }
  if (PROP_IS(property, VolumeMin))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            VolumeMin = (min + max) / 2.f;
            VolumeMinProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      VolumeMin = property->GetFloat();
      VolumeMinProperty = property;
    }
    return false;
  }
  if (PROP_IS(property, VolumeMax))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            VolumeMax = (min + max) / 2.f;
            VolumeMaxProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      VolumeMax = property->GetFloat();
      VolumeMaxProperty = property;;
    }
    return false;
  }
  if (PROP_IS(property, PitchMin))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            PitchMin = (min + max) / 2.f;
            PitchMinProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      PitchMin = property->GetFloat();
      PitchMinProperty = property;
    }
    return false;
  }
  if (PROP_IS(property, PitchMax))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            PitchMax = (min + max) / 2.f;
            PitchMaxProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      PitchMax = property->GetFloat();
      PitchMaxProperty = property;
    }
    return false;
  }
  if (PROP_IS(property, SoundSlots))
  {
    for (FPropertyValue* slotEntry : property->GetArray())
    {
      FAmbientSoundSlot& slot = SoundSlots.emplace_back();
      for (FPropertyValue* tmp : slotEntry->GetArray())
      {
        FPropertyValue* field = tmp->GetPropertyTag().Value;
        if (tmp->GetPropertyTag().Name == "Wave")
        {
          slot.Wave = Cast<USoundNodeWave>(field->GetObjectValuePtr());
        }
        else if (tmp->GetPropertyTag().Name == "PitchScale")
        {
          slot.PitchScale = field->GetFloat();
        }
        else if (tmp->GetPropertyTag().Name == "VolumeScale")
        {
          slot.VolumeScale = field->GetFloat();
        }
        else if (tmp->GetPropertyTag().Name == "Weight")
        {
          slot.Weight = field->GetFloat();
        }
      }
    }
  }
  return false;
}

void USoundNodeAmbient::PostLoad()
{
  if (GetPackage()->GetFileVersion() > VER_TERA_CLASSIC && !DistanceModelProperty)
  {
    DistanceModel = ATTENUATION_Linear;
  }
  Super::PostLoad();
}

void USoundNodeAmbient::GetWaves(std::vector<class USoundNodeWave*>& waves)
{
  if (Wave)
  {
    waves.emplace_back(Wave);
  }
  for (FAmbientSoundSlot& slot : SoundSlots)
  {
    if (slot.Wave)
    {
      waves.emplace_back(slot.Wave);
    }
  }
  Super::GetWaves(waves);
}

bool USoundNodeAttenuation::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bSpatialize);
  REGISTER_BOOL_PROP(bAttenuate);
  REGISTER_BOOL_PROP(bAttenuateWithLowPassFilter);
  REGISTER_BOOL_PROP(bAttenuateWithLPF);
  REGISTER_FLOAT_PROP(RadiusMin);
  REGISTER_FLOAT_PROP(RadiusMax);
  REGISTER_FLOAT_PROP(LPFRadiusMin);
  REGISTER_FLOAT_PROP(LPFRadiusMax);
  if (PROP_IS(property, DistanceModel))
  {
    DistanceModel = (SoundDistanceModel)property->GetByte();
    DistanceModelProperty = property;
    return true;
  }
  if (PROP_IS(property, MinRadius))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            MinRadius = (min + max) / 2.f;
            MinRadiusProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
    return false;
  }
  if (PROP_IS(property, MaxRadius))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            MaxRadius = (min + max) / 2.f;
            MaxRadiusProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
    return false;
  }
  if (PROP_IS(property, LPFMinRadius))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            LPFMinRadius = (min + max) / 2.f;
            LPFMinRadiusProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
    return false;
  }
  if (PROP_IS(property, LPFMaxRadius))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            LPFMaxRadius = (min + max) / 2.f;
            LPFMaxRadiusProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
    return false;
  }
  return false;
}

void USoundNodeAttenuation::PostLoad()
{
  if (GetPackage()->GetFileVersion() > VER_TERA_CLASSIC && !DistanceModelProperty)
  {
    DistanceModel = ATTENUATION_Linear;
  }
  Super::PostLoad();
}

bool USoundNodeAmbientNonLoop::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, DelayTime))
  {
    for (FPropertyValue* v : property->GetArray())
    {
      if (v->Field && v->Field->GetObjectName() == "Distribution")
      {
        FPropertyTag* tmp = v->GetPropertyTagPtr();
        if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
        {
          df->Load();
          float min = 0.f;
          float max = 0.f;
          df->GetOutRange(min, max);
          DelayTime = (min + max) / 2.f;
          DelayTimeProperty = property;
          return true;
        }
      }
    }
  }
  return false;
}

bool USoundNodeConcatenator::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, InputVolume))
  {
    for (FPropertyValue* v : property->GetArray())
    {
      InputVolume.emplace_back(v->GetFloat());
    }
    return true;
  }
  return false;
}

bool USoundNodeLooping::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bLoopIndefinitely);
  REGISTER_FLOAT_PROP(LoopCountMin);
  REGISTER_FLOAT_PROP(LoopCountMax);
  if (PROP_IS(property, LoopCount))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            LoopCountMin = min;
            LoopCountMax = max;
            LoopCountProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
  }
  return false;
}

bool USoundNodeMixer::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, InputVolume))
  {
    for (FPropertyValue* v : property->GetArray())
    {
      InputVolume.emplace_back(v->GetFloat());
    }
    return true;
  }
  return false;
}

bool USoundNodeModulator::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, PitchModulation))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            PitchMin = min;
            PitchMax = max;
            PitchModulationProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
    return false;
  }
  if (PROP_IS(property, VolumeModulation))
  {
    if (GetPackage()->GetFileVersion() == VER_TERA_CLASSIC)
    {
      for (FPropertyValue* v : property->GetArray())
      {
        if (v->Field && v->Field->GetObjectName() == "Distribution")
        {
          FPropertyTag* tmp = v->GetPropertyTagPtr();
          if (UDistributionFloat* df = Cast<UDistributionFloat>(tmp->Value->GetObjectValuePtr(false)))
          {
            df->Load();
            float min = 0.f;
            float max = 0.f;
            df->GetOutRange(min, max);
            VolumeMin = min;
            VolumeMax = max;
            VolumeModulationProperty = property;
            return true;
          }
        }
      }
    }
    else
    {
      DBreak();
    }
    return false;
  }
}

bool USoundNodeRandom::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bRandomizeWithoutReplacement);
  if (PROP_IS(property, Weights))
  {
    WeightsProperty = property;
    for (FPropertyValue* v : WeightsProperty->GetArray())
    {
      Weights.emplace_back(v->GetFloat());
    }
    return true;
  }
  return false;
}
