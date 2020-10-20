#pragma once
#include "UObject.h"
#include "FStream.h"

struct FRawAnimSequenceTrack {
  std::vector<FVector> PosKeys;
  std::vector<FQuat> RotKeys;

  friend FStream& operator<<(FStream& s, FRawAnimSequenceTrack& t);
  FILE_OFFSET PosKeysElementSize = sizeof(FVector);
  FILE_OFFSET RotKeysElementSize = sizeof(FQuat);
};

class UAnimSequence : public UObject {
public:
  DECL_UOBJ(UAnimSequence, UObject);

  ~UAnimSequence() override
  {
    free(SerializedData);
  }

  UPROP(FName, SequenceName, NAME_None);
  UPROP(float, SequenceDuration, 0.);
  UPROP(int32, NumFrames, 0);
  UPROP(bool, bNoLoopingInterpolation, false);
  UPROP(std::vector<int>, CompressedTrackOffsets, {});
  UPROP(AnimationCompressionFormat, RotationCompressionFormat, ACF_None);
  UPROP(AnimationCompressionFormat, TranslationCompressionFormat, ACF_None);

  bool RegisterProperty(FPropertyTag* property) override;

  void Serialize(FStream& s) override;

protected:
  std::vector<FRawAnimSequenceTrack> RawAnimationData;
  int32 NumBytes = 0;
  uint8* SerializedData = nullptr;
};