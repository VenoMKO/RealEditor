#include "UAnimation.h"
#include "FPackage.h"
#include "USkeletalMesh.h"
#include "UObjectRedirector.h"
#include "Cast.h"

const int32 CompressedTranslationStrides[] = { sizeof(float), sizeof(float), sizeof(float), sizeof(FVectorIntervalFixed32NoW), sizeof(float), sizeof(float), 0 };
const int32 CompressedTranslationNum[] = { 3, 3, 3, 1, 3, 3, 0 };
const int32 CompressedRotationStrides[] = { sizeof(float), sizeof(float), sizeof(uint16), sizeof(FQuatIntervalFixed32NoW), sizeof(FQuatFixed32NoW), sizeof(FQuatFloat32NoW), 0 };
const int32 CompressedRotationNum[] = { 4, 3, 3, 1, 1, 1, 0 };

void AligneReadStream(FStream& s, const int32 alignment)
{
  const int64 base = (int64)s.GetPosition();
  const int32 padding = static_cast<int32>(Align(base, alignment) - base);
  s.SetPosition(s.GetPosition() + padding);
}

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

bool UAnimSet::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bAnimRotationOnly);
  if (PROP_IS(property, PreviewSkelMeshName))
  {
    PreviewSkelMeshName = &property->GetName();
    PreviewSkelMeshNameProperty = property;
    return true;
  }
  if (PROP_IS(property, TrackBoneNames))
  {
    TrackBoneNames = &property->GetArray();
    TrackBoneNamesProperty = property;
    return true;
  }
  if (PROP_IS(property, Sequences))
  {
    Sequences = &property->GetArray();
    SequencesProperty = property;
    return true;
  }
  return false;
}

USkeletalMesh* UAnimSet::GetPreviewSkeletalMesh() const
{
  if (!PreviewSkelMeshName)
  {
    return nullptr;
  }
  USkeletalMesh* mesh = nullptr;
  FString objectPath = PreviewSkelMeshName->String();
  FString packageName = objectPath.Split('.').front();
  try
  {
    if (std::shared_ptr<FPackage> pkg = FPackage::GetPackageNamed(packageName))
    {
      pkg->Load();
      if (UObject* obj = pkg->GetObject(objectPath))
      {
        obj->Load();
        mesh = Cast<USkeletalMesh>(obj);
        if (!mesh)
        {
          if (UObjectRedirector* redirector = Cast<UObjectRedirector>(obj))
          {
            mesh = Cast<USkeletalMesh>(redirector->GetObject());
          }
        }
      }
      if (mesh)
      {
        GetPackage()->RetainPackage(pkg);
      }
      else
      {
        FPackage::UnloadPackage(pkg);
      }
    }
  }
  catch (...)
  {
  }
  return mesh;
}

float UAnimSet::GetSkeletalMeshMatchRatio(USkeletalMesh* mesh) const
{
  if (!mesh)
  {
    return 0;
  }
  auto bones = mesh->GetReferenceSkeleton();
  auto tracks = GetTrackBoneNames();
  int32 result = 0;
  for (const FName& name : tracks)
  {
    if (std::find_if(bones.begin(), bones.end(), [&](const FMeshBone& b) { return b.Name == name; }) != bones.end())
    {
      result++;
    }
  }
  if (!result || !tracks.size())
  {
    return 0;
  }
  return float(result) / float(tracks.size());
}

std::vector<FName> UAnimSet::GetTrackBoneNames() const
{
  std::vector<FName> result;
  if (!TrackBoneNames)
  {
    return result;
  }
  for (FPropertyValue* v : *TrackBoneNames)
  {
    result.emplace_back(v->GetName());
  }
  return result;
}

std::vector<UAnimSequence*> UAnimSet::GetSequences() const
{
  std::vector<UAnimSequence*> result;
  if (!Sequences)
  {
    return result;
  }
  for (FPropertyValue* v : *Sequences)
  {
    if (UAnimSequence* s = Cast<UAnimSequence>(v->GetObjectValuePtr()))
    {
      result.emplace_back(s);
    }
  }
  return result;
}

bool UAnimSequence::RegisterProperty(FPropertyTag* property)
{
  if (PROP_IS(property, SequenceName))
  {
    SequenceName = property->Value->GetName();
    SequenceNameProperty = property;
    return true;
  }
  if (PROP_IS(property, SequenceLength))
  {
    SequenceLength = property->Value->GetFloat();
    SequenceLengthProperty = property;
    return true;
  }
  if (PROP_IS(property, RateScale))
  {
    RateScale = property->Value->GetFloat();
    RateScaleProperty = property;
    return true;
  }
  if (PROP_IS(property, NumFrames))
  {
    NumFrames = property->Value->GetInt();
    NumFramesProperty = property;
    return true;
  }
  if (PROP_IS(property, KeyEncodingFormat))
  {
    KeyEncodingFormat = (AnimationKeyFormat)property->Value->GetByte();
    KeyEncodingFormatProperty = property;
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

void UAnimSequence::GetTracks(std::vector<FTranslationTrack>& transTracks, std::vector<FRotationTrack>& rotTracks)
{
  if (TransTracks.empty() && RotTracks.empty())
  {
    SeparateRawDataIntoTracks(TransTracks, RotTracks);
  }
  if (TransTracks.empty() && RotTracks.empty())
  {
    if (UncompressedRotTracks.empty() && UncompressedTransTracks.empty())
    {
      UncompressTracks();
    }
    transTracks = UncompressedTransTracks;
    rotTracks = UncompressedRotTracks;
    return;
  }
  transTracks = TransTracks;
  rotTracks = RotTracks;
}

bool UAnimSequence::GetBoneTransform(USkeletalMesh* mesh, const FName& name, int32 frame, FVector& position, FQuat& orientation)
{
  int32 boneIndex = -1;
  auto* namesArrPtr = GetTypedOuter<UAnimSet>()->TrackBoneNames;
  if (!namesArrPtr)
  {
    return false;
  }
  for (int32 idx = 0; idx < namesArrPtr->size(); ++idx)
  {
    if (namesArrPtr->at(idx)->GetName() == name)
    {
      boneIndex = idx;
      break;
    }
  }
  if (boneIndex == -1)
  {
    return false;
  }

  if (TransTracks.empty() && RotTracks.empty())
  {
    SeparateRawDataIntoTracks(TransTracks, RotTracks);
  }

  if (TransTracks.size() || RotTracks.size())
  {
    size_t rotTrackIndex = std::min<size_t>(RotTracks[boneIndex].RotKeys.size() - 1, frame);
    size_t transTrackIndex = std::min<size_t>(TransTracks[boneIndex].PosKeys.size() - 1, frame);

    orientation = RotTracks[boneIndex].RotKeys[rotTrackIndex];
    if (GetTypedOuter<UAnimSet>()->bAnimRotationOnly && mesh)
    {
      std::vector<FMeshBone> refBones = mesh->GetReferenceSkeleton();
      for (FMeshBone& refBone : refBones)
      {
        if (refBone.Name == name)
        {
          if (refBone.ParentIndex == refBones[refBone.ParentIndex].ParentIndex)
          {
            // Keep root transform regardless of the bAnimRotationOnly flag
            position = TransTracks[boneIndex].PosKeys[transTrackIndex];
            break;
          }
          position = refBone.BonePos.Position;
          break;
        }
      }
    }
    else
    {
      position = TransTracks[boneIndex].PosKeys[transTrackIndex];
    }
    return true;
  }
#ifdef _DEBUG
  if (UncompressedRotTracks.empty() && UncompressedTransTracks.empty())
  {
    UncompressTracks();
  }
  if (UncompressedRotTracks.size() || UncompressedTransTracks.size())
  {
    size_t rotTrackIndex = std::min<size_t>(UncompressedRotTracks[boneIndex].RotKeys.size() - 1, frame);
    size_t transTrackIndex = std::min<size_t>(UncompressedTransTracks[boneIndex].PosKeys.size() - 1, frame);

    orientation = UncompressedRotTracks[boneIndex].RotKeys[rotTrackIndex];
    if (GetTypedOuter<UAnimSet>()->bAnimRotationOnly && mesh)
    {
      std::vector<FMeshBone> refBones = mesh->GetReferenceSkeleton();
      for (FMeshBone& refBone : refBones)
      {
        if (refBone.Name == name)
        {
          position = refBone.BonePos.Position;
          break;
        }
      }
    }
    else
    {
      position = UncompressedTransTracks[boneIndex].PosKeys[transTrackIndex];
    }
    return true;
  }
#endif
  return false;
}

int32 UAnimSequence::GetRawFramesCount()
{
  if (NumRawFrames >= 0)
  {
    return NumRawFrames;
  }
  if (TransTracks.empty() && RotTracks.empty())
  {
    SeparateRawDataIntoTracks(TransTracks, RotTracks);
  }
  NumRawFrames = 0;
  for (const FTranslationTrack& t : TransTracks)
  {
    NumRawFrames = std::max<int32>(t.PosKeys.size(), NumRawFrames);
  }
  for (const FRotationTrack& t : RotTracks)
  {
    NumRawFrames = std::max<int32>(t.RotKeys.size(), NumRawFrames);
  }
  return NumRawFrames;
}

double UAnimSequence::GetFrameRate()
{
  double v = double(GetRawFramesCount() - 1) / double(SequenceLength) * double(RateScale);
  return v != 0. ? v : 1.;
}

void UAnimSequence::SeparateRawDataIntoTracks(std::vector<FTranslationTrack>& OutTranslationData, std::vector<FRotationTrack>& OutRotationData)
{
  const int32 NumTracks = RawAnimationData.size();
  OutTranslationData.resize(NumTracks);
  OutRotationData.resize(NumTracks);

  for (int32 TrackIndex = 0; TrackIndex < NumTracks; ++TrackIndex)
  {
    const FRawAnimSequenceTrack& RawTrack = RawAnimationData[TrackIndex];
    FTranslationTrack& TranslationTrack = OutTranslationData[TrackIndex];
    FRotationTrack& RotationTrack = OutRotationData[TrackIndex];
    TranslationTrack.Name = GetTypedOuter<UAnimSet>()->TrackBoneNames->at(TrackIndex)->GetName();
    RotationTrack.Name = GetTypedOuter<UAnimSet>()->TrackBoneNames->at(TrackIndex)->GetName();

    const int32 PrevNumPosKeys = RawTrack.PosKeys.size();
    const int32 PrevNumRotKeys = RawTrack.RotKeys.size();

    if (PrevNumPosKeys == 0 || PrevNumRotKeys == 0)
    {
      continue;
    }

    for (const auto& PosKey : RawTrack.PosKeys)
    {
      TranslationTrack.PosKeys.emplace_back(PosKey);
    }

    for (const auto& RotKey : RawTrack.RotKeys)
    {
      RotationTrack.RotKeys.emplace_back(RotKey);
    }
  }
}

void UAnimSequence::UncompressTracks()
{
  // Unstable, not fully implemented yet.
#ifdef _DEBUG
  MReadStream s(SerializedData, false, NumBytes);
  if (KeyEncodingFormat != AKF_PerTrackCompression)
  {
    const int32 tStride = CompressedTranslationStrides[static_cast<AnimationCompressionFormat>(TranslationCompressionFormat)];
    const int32 rStride = CompressedRotationStrides[static_cast<AnimationCompressionFormat>(RotationCompressionFormat)];

    const int32 tracksCount = CompressedTrackOffsets.size() / 4;

    for (int32 trackIndex = 0; trackIndex < tracksCount; ++trackIndex)
    {
      const int32 tOffset = CompressedTrackOffsets[trackIndex * 4 + 0];
      const int32 tKeysCount = CompressedTrackOffsets[trackIndex * 4 + 1];
      const int32 rOffset = CompressedTrackOffsets[trackIndex * 4 + 2];
      const int32 rKeysCount = CompressedTrackOffsets[trackIndex * 4 + 3];

      std::vector<FVector> translations;
      std::vector<FQuat> orientations;
      std::vector<uint16> times;

      // Translations
      if (tKeysCount != INDEX_NONE)
      {
        s.SetPosition(tOffset);
        const int32 format = (tKeysCount == 1) ? ACF_None : TranslationCompressionFormat;

        if (format == ACF_IntervalFixed32NoW)
        {
          float mins[3];
          float ranges[3];
          s.SerializeBytes(mins, sizeof(mins));
          s.SerializeBytes(ranges, sizeof(ranges));
          for (int32 keyIdx = 0; keyIdx < tKeysCount; ++keyIdx)
          {
            FVectorIntervalFixed32NoW v;
            s << v;
            v.ToVector(translations.emplace_back(), mins, ranges);
          }
        }
        else
        {
          DBreakIf(format == ACF_Identity);
          for (int32 keyIdx = 0; keyIdx < tKeysCount; ++keyIdx)
          {
            s << translations.emplace_back();
          }
        }

        if (KeyEncodingFormat == AKF_VariableKeyLerp && tKeysCount > 1)
        {
          AligneReadStream(s, 4);
          for (int32 idx = 0; idx < tKeysCount; ++idx)
          {
            if (NumFrames > 0xFF)
            {
              uint16 tmp;
              s << tmp;
              times.emplace_back(tmp);
            }
            else
            {
              uint8 tmp;
              s << tmp;
              times.emplace_back(uint16(tmp));
            }
          }
        }
      }

      AligneReadStream(s, 4);

      // Rotations
      if (rKeysCount != INDEX_NONE)
      {
        s.SetPosition(rOffset);
        const int32 format = (tKeysCount == 1) ? ACF_Float96NoW : RotationCompressionFormat;
        float mins[3];
        float ranges[3];
        if (format == ACF_IntervalFixed32NoW)
        {
          s.SerializeBytes(mins, sizeof(mins));
          s.SerializeBytes(ranges, sizeof(ranges));

          for (int32 idx = 0; idx < rKeysCount; ++idx)
          {
            FQuatIntervalFixed32NoW q;
            s << q;
            FQuat r;
            q.ToQuat(r, mins, ranges);
            orientations.emplace_back(r);
          }
        }
        else if (format == ACF_Float96NoW)
        {
          for (int32 idx = 0; idx < rKeysCount; ++idx)
          {
            FQuatFloat96NoW q;
            s << q;
            FQuat r;
            q.ToQuat(r);
            orientations.emplace_back(r);
          }
        }
        else if (format == ACF_Fixed48NoW)
        {
          for (int32 idx = 0; idx < rKeysCount; ++idx)
          {
            FQuatFixed48NoW q;
            s << q;
            FQuat r;
            q.ToQuat(r);
            orientations.emplace_back(r);
          }
        }
        else if (format == ACF_Fixed32NoW)
        {
          for (int32 idx = 0; idx < rKeysCount; ++idx)
          {
            FQuatFixed32NoW q;
            s << q;
            FQuat r;
            q.ToQuat(r);
            orientations.emplace_back(r);
          }
        }
        else if (format == ACF_Float32NoW)
        {
          for (int32 idx = 0; idx < rKeysCount; ++idx)
          {
            FQuatFloat32NoW q;
            s << q;
            FQuat r;
            q.ToQuat(r);
            orientations.emplace_back(r);
          }
        }
        else if (format == ACF_None)
        {
          for (int32 idx = 0; idx < rKeysCount; ++idx)
          {
            FQuat q;
            s << q;
            orientations.emplace_back(q);
          }
        }
        else
        {
          DBreak();
        }
      }

      AligneReadStream(s, 4);

      if (translations.size())
      {
        FTranslationTrack t;
        t.PosKeys = translations;
        if (times.size())
        {
          // DBreak();
        }
        UncompressedTransTracks.emplace_back(t);
      }
      if (orientations.size())
      {
        FRotationTrack t;
        t.RotKeys = orientations;
        UncompressedRotTracks.emplace_back(t);
      }
    }
  }
  else
  {
    const int32 NumTracks = CompressedTrackOffsets.size() / 2;
  }
#endif
}
