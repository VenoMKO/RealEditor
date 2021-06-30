#pragma once
#include "UObject.h"
#include "FStream.h"
#include "FName.h"

struct FRawAnimSequenceTrack {
  std::vector<FVector> PosKeys;
  std::vector<FQuat> RotKeys;

  friend FStream& operator<<(FStream& s, FRawAnimSequenceTrack& t);
  FILE_OFFSET PosKeysElementSize = sizeof(FVector);
  FILE_OFFSET RotKeysElementSize = sizeof(FQuat);
};

struct FTranslationTrack {
  FName Name;
  std::vector<FVector> PosKeys;
};

struct FRotationTrack {
  FName Name;
  std::vector<FQuat> RotKeys;
};

class UAnimSet : public UObject {
public:
  DECL_UOBJ(UAnimSet, UObject);

  UPROP_CREATABLE(bool, bAnimRotationOnly, false);
  UPROP_CREATABLE_ARR_PTR(TrackBoneNames);
  UPROP_CREATABLE_ARR_PTR(Sequences);
  UPROP(FName*, PreviewSkelMeshName, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;

  class USkeletalMesh* GetPreviewSkeletalMesh() const;

  float GetSkeletalMeshMatchRatio(class USkeletalMesh* mesh) const;

  std::vector<FName> GetTrackBoneNames() const;

  std::vector<class UAnimSequence*> GetSequences() const;
};

class UAnimSequence : public UObject {
public:
  DECL_UOBJ(UAnimSequence, UObject);

  ~UAnimSequence()
  {
    free(SerializedData);
  }

  UPROP(FName, SequenceName, NAME_None);
  UPROP(float, SequenceLength, 0.);
  UPROP(float, RateScale, 1.);
  UPROP(int32, NumFrames, 0);
  UPROP(bool, bNoLoopingInterpolation, false);
  UPROP(std::vector<int>, CompressedTrackOffsets, {});
  UPROP(AnimationCompressionFormat, RotationCompressionFormat, ACF_Float96NoW);
  UPROP(AnimationCompressionFormat, TranslationCompressionFormat, ACF_None);
  UPROP(AnimationKeyFormat, KeyEncodingFormat, AKF_ConstantKeyLerp);

  bool RegisterProperty(FPropertyTag* property) override;

  void Serialize(FStream& s) override;

  void GetTracks(std::vector<FTranslationTrack>& transTracks, std::vector<FRotationTrack>& rotTracks);

  bool GetBoneTransform(USkeletalMesh* mesh, const FName& name, int32 frame, FVector& position, FQuat& orientation);

  int32 GetRawFramesCount();

  double GetFrameRate();

protected:
  void SeparateRawDataIntoTracks(std::vector<FTranslationTrack>& OutTranslationData, std::vector<FRotationTrack>& OutRotationData);
  void UncompressTracks();

protected:
  std::vector<FRawAnimSequenceTrack> RawAnimationData;
  std::vector<FTranslationTrack> TransTracks;
  std::vector<FRotationTrack> RotTracks;
  std::vector<FTranslationTrack> UncompressedTransTracks;
  std::vector<FRotationTrack> UncompressedRotTracks;
  int32 NumBytes = 0;
  int32 NumRawFrames = -1;
  uint8* SerializedData = nullptr;
};