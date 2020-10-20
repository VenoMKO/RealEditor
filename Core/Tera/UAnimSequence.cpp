#include "UAnimSequence.h"

FStream& operator<<(FStream& s, FRawAnimSequenceTrack& t)
{
  s << t.PosKeysElementSize;
  int32 cnt = (int32)t.PosKeys.size();
  s << cnt;
  if (s.IsReading())
  {
    t.PosKeys.resize(cnt);
  }
  for (int32 idx = 0; idx < cnt; ++idx)
  {
    s << t.PosKeys[idx];
  }
  s << t.RotKeysElementSize;
  cnt = (int32)t.RotKeys.size();
  s << cnt;
  if (s.IsReading())
  {
    t.RotKeys.resize(cnt);                       
  }
  for (int32 idx = 0; idx < cnt; ++idx)
  {
    s << t.RotKeys[idx];
  }
  return s;
}

bool UAnimSequence::RegisterProperty(FPropertyTag* property)
{
  if (PROP_IS(property, SequenceName))
  {
    SequenceName = property->Value->GetName();
    SequenceNameProperty = property;
    return true;
  }
  if (PROP_IS(property, SequenceDuration))
  {
    SequenceDuration = property->Value->GetFloat();
    SequenceDurationProperty = property;
    return true;
  }
  if (PROP_IS(property, NumFrames))
  {
    NumFrames = property->Value->GetInt();
    NumFramesProperty = property;
    return true;
  }
  if (PROP_IS(property, CompressedTrackOffsets))
  {
    if (property->Value && property->Value->Data)
    {
      CompressedTrackOffsets.clear();
      const std::vector<FPropertyValue*> values = property->Value->GetArray();
      for (FPropertyValue* v : values)
      {
        CompressedTrackOffsets.push_back(v->GetInt());
      }
      CompressedTrackOffsetsProperty = property;
    }
    
    return true;
  }
  if (PROP_IS(property, RotationCompressionFormat))
  {
    RotationCompressionFormat = (AnimationCompressionFormat)property->Value->GetByte();
    RotationCompressionFormatProperty = property;
    return true;
  }
  if (PROP_IS(property, TranslationCompressionFormat))
  {
    TranslationCompressionFormat = (AnimationCompressionFormat)property->Value->GetByte();
    TranslationCompressionFormatProperty = property;
    return true;
  }
  if (PROP_IS(property, bNoLoopingInterpolation))
  {
    bNoLoopingInterpolation = property->Value->GetBool();
    bNoLoopingInterpolationProperty = property;
    return true;
  }
  return false;
}

void UAnimSequence::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << RawAnimationData;
  s << NumBytes;
  if (s.IsReading())
  {
    free(SerializedData);
    SerializedData = (uint8*)malloc(NumBytes);
  }
  s.SerializeBytes(SerializedData, NumBytes);
}
