#include "SoundTravaller.h"
#include <Tera/USoundNode.h>
#include <vorbis/vorbisfile.h>


static size_t OggRead(void* ptr, size_t size, size_t nmemb, void* datasource)
{
  SoundTravaller* travaller = (SoundTravaller*)datasource;
  return travaller->ReadOgg(ptr, size * nmemb);
}

static int32 OggSeek(void* datasource, ogg_int64_t offset, int whence)
{
  SoundTravaller* travaller = (SoundTravaller*)datasource;
  return travaller->SeekOgg(offset, whence);
}

static int32 OggClose(void* datasource)
{
  return 0;
}

static long OggTell(void* datasource)
{
  SoundTravaller* travaller = (SoundTravaller*)datasource;
  return travaller->TellOgg();
}

bool SoundTravaller::Visit(USoundNodeWave* wave)
{
  if (!wave)
  {
    Error = "No object provided!";
    return false;
  }
  if (!DataSize || !Data)
  {
    Error = "OGG file is empty or corrupted!";
    return false;
  }

  FSoundInfo info;
  GetOggInfo(info);

  if (info.Duration <= 0)
  {
    Error = "Invalid OGG length!";
    return false;
  }

  wave->PCData.Realloc(DataSize);
  wave->PCData.ElementCount = DataSize;
  memcpy(wave->PCData.GetAllocation(), Data, DataSize);
  wave->PCData.BulkDataFlags = BULKDATA_None;
  free(wave->ResourceData);
  wave->ResourceData = nullptr;

  if (wave->SampleRateProperty)
  {
    wave->SampleRate = info.SampleRate;
    wave->SampleRateProperty->Value->GetInt() = wave->SampleRate;
  }

  if (wave->DurationProperty)
  {
    wave->Duration = info.Duration;
    wave->DurationProperty->Value->GetFloat() = wave->Duration;
  }

  if (wave->NumChannelsProperty)
  {
    wave->NumChannels = info.NumChannels;
    wave->NumChannelsProperty->Value->GetInt() = wave->NumChannels;
  }

  wave->PostLoad();
  wave->MarkDirty();
  return true;
}

size_t SoundTravaller::ReadOgg(void* ptr, uint32 size)
{
  size_t sizeToRead = std::min<size_t>(size, DataSize - DataOffset);
  memcpy(ptr, (uint8*)Data + DataOffset, sizeToRead);
  DataOffset += sizeToRead;
  return sizeToRead;
}

int32 SoundTravaller::SeekOgg(uint64 offset, int whence)
{
  switch (whence)
  {
  case SEEK_SET:
    DataOffset = offset;
    break;

  case SEEK_CUR:
    DataOffset += offset;
    break;

  case SEEK_END:
    DataOffset = DataSize - offset;
    break;
  }

  return DataOffset;
}

uint32 SoundTravaller::TellOgg()
{
  return DataOffset;
}

bool SoundTravaller::GetOggInfo(FSoundInfo& info)
{
  OggVorbis_File vf;
  ov_callbacks cb;
  cb.read_func = OggRead;
  cb.seek_func = OggSeek;
  cb.close_func = OggClose;
  cb.tell_func = OggTell;

  DataOffset = 0;
  if (ov_open_callbacks(this, &vf, nullptr, 0, cb) < 0)
  {
    Error = "Not a valid OGG file!";
    return false;
  }

  if (vorbis_info* vi = ov_info(&vf, -1))
  {
    info.SampleRate = vi->rate;
    info.NumChannels = vi->channels;
    info.Duration = ov_time_total(&vf, -1);

    ov_clear(&vf);
    return true;
  }
  ov_clear(&vf);

  Error = "Not a valid OGG file!";
  return false;
}
