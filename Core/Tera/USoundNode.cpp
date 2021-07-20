#include "USoundNode.h"

bool USoundNodeWave::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return false;
  }
  if (PROP_IS(property, Duration))
  {
    Duration = property->Value->GetFloat();
    DurationProperty = property;
    return true;
  }
  else if (PROP_IS(property, NumChannels))
  {
    NumChannels = property->Value->GetInt();
    NumChannelsProperty = property;
    return true;
  }
  else if (PROP_IS(property, SampleRate))
  {
    SampleRate = property->Value->GetInt();
    SampleRateProperty = property;
    return true;
  }
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
