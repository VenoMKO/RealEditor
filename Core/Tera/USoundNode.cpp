#include "USoundNode.h"
#include "UComponent.h"
#include "FPackage.h"
#include "UClass.h"
#include "Cast.h"

#include <Utils/AConfiguration.h>

int32 ExportSoundCueNodeRecursive(FString& cueString, USoundNode* node, int32& index, bool loop, float scale)
{
  index++;
  FString c = node->GetClassNameString();
  FString childStr;
  int32 currentIndex = index++;
  if (c == USoundNodeAmbientNonLoop::StaticClassName())
  {
    // This one is a bit confusing.
    // While the node clearly has a "NonLoop" in its name, it actually does sound looping.
    // The only difference is that looping is NOT seamless.
    // RE create a separate loop node to mimic this behavior and
    // turns off bLooping flag of child wave nodes to prevent seamless looping.
    USoundNodeAmbientNonLoop* tnode = Cast<USoundNodeAmbientNonLoop>(node);
    loop = false;
    cueString += FString::Sprintf("%d\tSoundNodeLoop\n", currentIndex);
    cueString += FString::Sprintf("\tChildren=((Index=%d))\n", index);
    if (tnode->DelayMin || tnode->DelayMax)
    {
      cueString += FString::Sprintf("%d\tSoundNodeDelay\n", index);
      index++;
      cueString += FString::Sprintf("\tDelayMin=%.06f\n\tDelayMax=%.06f\n", tnode->DelayMin, tnode->DelayMax);
      cueString += FString::Sprintf("\tChildren=((Index=%d))\n", index);
    }
    cueString += FString::Sprintf("%d\t%s\n", index, USoundNodeRandom::StaticClassName());
    cueString += "\tChildren=(";
    for (int32 sIdx = 0; sIdx < tnode->SoundSlots.size(); ++sIdx)
    {
      const FAmbientSoundSlot& childNode = tnode->SoundSlots[sIdx];
      if (!childNode.Wave)
      {
        continue;
      }
      int32 childIdx = ExportSoundCueNodeRecursive(childStr, childNode.Wave, index, loop, scale);
      cueString += FString::Sprintf("(Index=%d,Weight=%.06f)", childIdx, childNode.Weight);
      if (sIdx + 1 < tnode->SoundSlots.size())
      {
        cueString += ",";
      }
    }
    cueString += ")\n";
  }
  else if (c == USoundNodeAmbient::StaticClassName())
  {
    USoundNodeAmbient* amb = Cast<USoundNodeAmbient>(node);
    if (USoundNodeWave* wave = amb->Wave)
    {
      currentIndex = ExportSoundCueNodeRecursive(childStr, wave, index, loop, scale);
    }
  }
  else if (c == USoundNodeLooping::StaticClassName())
  {
    USoundNodeLooping* typedNode = Cast<USoundNodeLooping>(node);
    if (!typedNode->bLoopIndefinitely)
    {
      cueString += "\tbLoopIndefinitely=False\n";
      cueString += FString::Sprintf("\tLoopCount=%d\n", (typedNode->LoopCountMin + typedNode->LoopCountMin) / 2);
    }
    if (node->ChildNodes.size() > 1)
    {
      cueString += FString::Sprintf("%d\t%s\n", currentIndex, USoundNodeRandom::StaticClassName());
      cueString += "\tChildren=(";
      for (int32 cIdx = 0; cIdx < node->ChildNodes.size(); ++cIdx)
      {
        int32 childIndex = ExportSoundCueNodeRecursive(childStr, node->ChildNodes[cIdx], index, true, scale);
        cueString = FString::Sprintf("(Index=%d)", childIndex);
        if (cIdx + 1 < node->ChildNodes.size())
        {
          cueString += ",";
        }
      }
      cueString += ")\n";
    }
    else if (node->ChildNodes.size() == 1)
    {
      currentIndex = ExportSoundCueNodeRecursive(cueString, node->ChildNodes[0], index, true, scale);
    }
  }
  else if (c == USoundNodeMixer::StaticClassName())
  {
    cueString += FString::Sprintf("%d\t%s\n", currentIndex, USoundNodeMixer::StaticClassName());
    cueString += "\tChildren=(";
    USoundNodeMixer* mixer = Cast<USoundNodeMixer>(node);
    if (mixer->InputVolume.empty())
    {
      mixer->InputVolume.emplace_back(1);
    }
    for (int32 cIdx = 0; cIdx < node->ChildNodes.size(); ++cIdx)
    {
      int32 childIndex = ExportSoundCueNodeRecursive(childStr, mixer->ChildNodes[cIdx], index, loop, scale);
      cueString += FString::Sprintf("(Index=%d,Volume=%.06f)", childIndex, mixer->InputVolume[cIdx % mixer->InputVolume.size()]);
      if (cIdx + 1 < node->ChildNodes.size())
      {
        cueString += ",";
      }
    }
    cueString += ")\n";
  }
  else if (c == USoundNodeModulator::StaticClassName())
  {
    USoundNodeModulator* tnode = Cast<USoundNodeModulator>(node);
    cueString += FString::Sprintf("%d\t%s\n", currentIndex, USoundNodeModulator::StaticClassName());
    cueString += FString::Sprintf("\tPitchMin=%.06f\n", tnode->PitchMin);
    cueString += FString::Sprintf("\tPitchMax=%.06f\n", tnode->PitchMax);
    cueString += FString::Sprintf("\tVolumeMin=%.06f\n", tnode->VolumeMin);
    cueString += FString::Sprintf("\tVolumeMax=%.06f\n", tnode->VolumeMax);

    if (tnode->ChildNodes.size() == 1)
    {
      int32 childIndex = ExportSoundCueNodeRecursive(childStr, tnode->ChildNodes[0], index, loop, scale);
      cueString += FString::Sprintf("\tChildren=((Index=%d))\n", childIndex);
    }
    else
    {
      cueString += "\tChildren=(";
      for (int32 cIdx = 0; cIdx < tnode->ChildNodes.size(); ++cIdx)
      {
        int32 childIndex = ExportSoundCueNodeRecursive(childStr, tnode->ChildNodes[cIdx], index, loop, scale);
        cueString += FString::Sprintf("(Index=%d)", childIndex);
        if (cIdx + 1 < tnode->ChildNodes.size())
        {
          cueString += ",";
        }
      }
    }
  }
  else if (c == USoundNodeRandom::StaticClassName())
  {
    cueString += FString::Sprintf("%d\t%s\n", currentIndex, USoundNodeRandom::StaticClassName());
    USoundNodeRandom* tnode = Cast<USoundNodeRandom>(node);
    if (tnode->Weights.empty())
    {
      tnode->Weights.emplace_back(1);
    }
    cueString += "\tChildren=(";
    for (int32 sIdx = 0; sIdx < tnode->ChildNodes.size(); ++sIdx)
    {
      int32 childIndex = ExportSoundCueNodeRecursive(childStr, tnode->ChildNodes[sIdx], index, loop, scale);
      cueString += FString::Sprintf("(Index=%d,Weight=%.06f)", childIndex, tnode->Weights[sIdx % tnode->Weights.size()]);
      if (sIdx + 1 < tnode->ChildNodes.size())
      {
        cueString += ",";
      }
    }
    cueString += ")\n";
  }
  else if (c == USoundNodeWave::StaticClassName())
  {
    cueString += FString::Sprintf("%d\t%s\n", currentIndex, USoundNodeWave::StaticClassName());
    cueString += FString::Sprintf("\tLooping=%s\n", loop ? "True" : "False");
    FString assetPath = FString::Sprintf("S1Game/%s", node->GetLocalDir(true, "/").UTF8().c_str());
    cueString += FString::Sprintf("\tSound=%s\n", assetPath.UTF8().c_str());
  }
  else if (c == USoundNodeAttenuation::StaticClassName())
  {
    USoundNodeAttenuation* tnode = Cast<USoundNodeAttenuation>(node);
    cueString += FString::Sprintf("%d\t%s\n", currentIndex, USoundNodeAttenuation::StaticClassName());
    cueString += FString::Sprintf("\tSpatialize=%s\n", tnode->bSpatialize ? "True" : "False");
    cueString += FString::Sprintf("\tAttenuate=%s\n", tnode->bAttenuate ? "True" : "False");
    cueString += FString::Sprintf("\tRadiusMin=%.06f\n", tnode->GetRadiusMin() * scale);
    cueString += FString::Sprintf("\tRadiusMax=%.06f\n", tnode->GetRadiusMax() * scale);
    cueString += FString::Sprintf("\tAttenuateWithLPF=%s\n", tnode->GetAttenuateWithLPF() ? "True" : "False");
    if (tnode->GetAttenuateWithLPF())
    {
      cueString += FString::Sprintf("\tLPFRadiusMin=%.06f\n", tnode->GetLPFRadiusMin() * scale);
      cueString += FString::Sprintf("\tLPFRadiusMax=%.06f\n", tnode->GetLPFRadiusMax() * scale);
    }
    FString distanceModel = "Linear";
    switch (tnode->DistanceModel)
    {
    case ATTENUATION_Logarithmic:
      distanceModel = "Logarithmic";
      break;
    case ATTENUATION_Inverse:
      distanceModel = "Inverse";
      break;
    case ATTENUATION_LogReverse:
      distanceModel = "LogReverse";
      break;
    case ATTENUATION_NaturalSound:
      distanceModel = "NaturalSound";
      break;
    }
    cueString += FString::Sprintf("\tDistanceModel=%s\n", distanceModel.UTF8().c_str());
    cueString += "\tChildren=(";
    for (int32 sIdx = 0; sIdx < tnode->ChildNodes.size(); ++sIdx)
    {
      int32 childIndex = ExportSoundCueNodeRecursive(childStr, tnode->ChildNodes[sIdx], index, loop, scale);
      cueString += FString::Sprintf("(Index=%d)", childIndex);
      if (sIdx + 1 < tnode->ChildNodes.size())
      {
        cueString += ",";
      }
    }
    cueString += ")\n";
  }
  else
  {
    DBreak();
  }
  if (childStr.Size())
  {
    cueString += childStr;
  }
  return currentIndex;
}

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

void USoundNodeWave::GetWaves(std::vector<USoundNodeWave*>& waves)
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
  REGISTER_INT_PROP(MaxConcurrentPlayCount);
  return false;
}

void USoundCue::GetWaves(std::vector<USoundNodeWave*>& waves)
{
  if (FirstNode)
  {
    FirstNode->GetWaves(waves);
  }
}

FString USoundCue::ExportCueToText(bool loop, float attScale)
{
  Load();
  if (!FirstNode)
  {
    return {};
  }
  if (attScale < 0.)
  {
    FMapExportConfig cfg;
    attScale = cfg.GlobalScale;
  }

  FString cueData = GetLocalDir(true, "/") + "\n";
  cueData += FString::Sprintf("Volume=%.06f\n", VolumeMultiplier);
  if (PitchMultiplier != 1.f)
  {
    cueData += FString::Sprintf("Pitch=%.06f\n", PitchMultiplier);
  }
  if (MaxConcurrentPlayCount != 16)
  {
    cueData += FString::Sprintf("MaxConcurrentPlayCount=%d\n", MaxConcurrentPlayCount);
  }
  cueData += "Nodes:\n";
  int32 index = 0;
  ExportSoundCueNodeRecursive(cueData, FirstNode, index, loop, attScale);
  return cueData;
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
  return false;
}

void USoundNodeAmbient::GetWaves(std::vector<USoundNodeWave*>& waves)
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

float USoundNodeAmbient::GetRadiusMin() const
{
  return GetPackage()->GetFileVersion() == VER_TERA_CLASSIC ? MinRadius : RadiusMin;
}

float USoundNodeAmbient::GetRadiusMax() const
{
  return GetPackage()->GetFileVersion() == VER_TERA_CLASSIC ? MaxRadius : RadiusMax;
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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

bool USoundNodeAmbientNonLoop::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(DelayMin);
  REGISTER_FLOAT_PROP(DelayMax);
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
          if (!df->IsValid())
          {
            return true;
          }
          float min = 0.f;
          float max = 0.f;
          df->GetOutRange(min, max);
          DelayMin = min;
          DelayMax = max;
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
            if (!df->IsValid())
            {
              return true;
            }
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
  }
  return false;
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

bool UAmbientSound::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bAutoPlay);
  REGISTER_TOBJ_PROP(AudioComponent, UAudioComponent*);
  return false;
}

bool UAmbientSoundSimple::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(AmbientProperties, USoundNodeAmbient*);
  REGISTER_TOBJ_PROP(SoundCueInstance, USoundCue*);
  REGISTER_TOBJ_PROP(SoundNodeInstance, USoundNodeAmbient*);
  return false;
}

bool UAmbientSoundSimpleToggleable::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bCurrentlyPlaying);
  REGISTER_BOOL_PROP(bFadeOnToggle);
  REGISTER_BOOL_PROP(bIgnoreAutoPlay);
  REGISTER_FLOAT_PROP(FadeInDuration);
  REGISTER_FLOAT_PROP(FadeInVolumeLevel);
  REGISTER_FLOAT_PROP(FadeOutDuration);
  REGISTER_FLOAT_PROP(FadeOutVolumeLevel);
  return false;
}
