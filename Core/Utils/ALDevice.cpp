#include "ALDevice.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>

#include <Tera/Core.h>
#include <Tera/USoundNode.h>

ALenum SoundState2AL(ALSoundSource::SoundState st)
{
  switch (st)
  {
  case ALSoundSource::SoundState::PLAYING:
    return AL_PLAYING;
  case ALSoundSource::SoundState::PAUSED:
    return AL_PAUSED;
  }
  return AL_STOPPED;
}
ALSoundSource::SoundState AL2SoundState(ALenum st)
{
  switch (st)
  {
  case AL_PLAYING:
    return ALSoundSource::SoundState::PLAYING;
  case AL_PAUSED:
    return ALSoundSource::SoundState::PAUSED;
  }
  return ALSoundSource::SoundState::STOPPED;
}


static size_t OggRead(void* ptr, size_t size, size_t nmemb, void* datasource)
{
  ALSoundSource* src = (ALSoundSource*)datasource;
  return src->ReadOgg(ptr, size * nmemb);
}

static int32 OggSeek(void* datasource, ogg_int64_t offset, int whence)
{
  ALSoundSource* src = (ALSoundSource*)datasource;
  return src->SeekOgg(offset, whence);
}

static int32 OggClose(void* datasource)
{
  return 0;
}

static long OggTell(void* datasource)
{
  ALSoundSource* src = (ALSoundSource*)datasource;
  return src->TellOgg();
}

ALDevice::ALDevice(const char* deviceName)
{
  Alive.store(true);
  Device = alcOpenDevice(deviceName);
  if (!Device)
  {
    UThrow("Failed to initialize ALC device!");
  }
  Context = alcCreateContext(Device, nullptr);
  if (!Context)
  {
    alcCloseDevice(Device);
    UThrow("Failed to initialize ALC context!");
  }
  alcMakeContextCurrent(Context);
}

ALDevice::~ALDevice()
{
  if (StatePoller.joinable())
  {
    Alive.store(false);
    StatePoller.join();
  }
  for (ALSoundSource* src : Sounds)
  {
    delete src;
  }
  Sounds.clear();
  alcMakeContextCurrent(nullptr);
  alcDestroyContext(Context);
  alcCloseDevice(Device);
}

size_t ALDevice::AddSoundSource(ALSoundObserver* observer, class USoundNodeWave* sound)
{
  if (!sound)
  {
    return 0;
  }
  if (!StatePoller.joinable())
  {
    StatePoller = std::thread([&] {
      while (Alive.load())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        std::vector<ALSoundSource*> sounds = GetSources();
        for (size_t idx = 0; idx < sounds.size(); ++idx)
        {
          ALSoundSource* sound = sounds[idx];
          if (!sound || !sound->PcmData)
          {
            continue;
          }
          ALenum state;
          alGetSourcei(sound->Id, AL_SOURCE_STATE, &state);
          ALSoundSource::SoundState realState = AL2SoundState(state);
          if (realState == sound->GetState())
          {
            continue;
          }
          switch (realState)
          {
            case ALSoundSource::SoundState::PLAYING:
            {
              sound->SetState(ALSoundSource::SoundState::PLAYING);
              sound->Observer->OnSoundStarted(idx + 1);
              break;
            }
            case ALSoundSource::SoundState::PAUSED:
            {
              sound->SetState(ALSoundSource::SoundState::PAUSED);
              sound->Observer->OnSoundPaused(idx + 1);
              break;
            }
            case ALSoundSource::SoundState::STOPPED:
            {
              sound->SetState(ALSoundSource::SoundState::STOPPED);
              sound->Observer->OnSoundStopped(idx + 1);
              break;
            }
          }
        }
      }
    });
  }

  OggVorbis_File vf;
  ov_callbacks cb;
  cb.read_func = OggRead;
  cb.seek_func = OggSeek;
  cb.close_func = OggClose;
  cb.tell_func = OggTell;
  ALSoundSource* src = new ALSoundSource;
  src->Observer = observer;
  src->Sound = sound;
  if (ov_open_callbacks(src, &vf, nullptr, 0, cb) < 0)
  {
    delete src;
    return 0;
  }
  vorbis_info* vi = ov_info(&vf, -1);
  if (!vi)
  {
    delete src;
    ov_clear(&vf);
    return 0;
  }

  uint32 SampleRate = vi->rate;
  uint32 NumChannels = vi->channels;
  uint32 Duration = ov_time_total(&vf, -1);

  alGenBuffers(1, &src->BufferId);
  size_t data_len = ov_pcm_total(&vf, -1) * vi->channels * 2;
  src->PcmData = malloc(data_len);
  for (size_t size = 0, offset = 0, sel = 0; (size = ov_read(&vf, (char*)src->PcmData + offset, 4096, 0, 2, 1, (int*)&sel)) != 0; offset += size) {
    if (size < 0)
    {
      delete src;
      ov_clear(&vf);
      return 0;
    }
  }
  alBufferData(src->BufferId, vi->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, src->PcmData, data_len, vi->rate);
  ALenum err = alGetError();
  if (err != AL_NO_ERROR)
  {
    delete src;
    ov_clear(&vf);
    return 0;
  }
  alGenSources(1, &src->Id);
  alSourceQueueBuffers(src->Id, 1, &src->BufferId);
  ov_clear(&vf);
  size_t result = 0;
  {
    std::lock_guard<std::mutex> l(SoundsMutex);
    Sounds.emplace_back(src);
    result = Sounds.size();
  }
  return result;
}

void ALDevice::RemoveSoundSource(size_t id)
{
  if (!id)
  {
    return;
  }
  ALSoundSource* src = GetSource(id);
  alSourceStop(src->Id);
  alSourceUnqueueBuffers(src->Id, 1, &src->BufferId);
  free(src->PcmData);
  src->PcmData = nullptr;
}

ALSoundSource::SoundState ALDevice::GetSoundState(size_t id)
{
  if (!id)
  {
    return ALSoundSource::SoundState::STOPPED;
  }
  ALSoundSource* src = GetSource(id);
  return src ? src->GetState() : ALSoundSource::SoundState::STOPPED;
}

bool ALDevice::Play(size_t id)
{
  if (!id)
  {
    return false;
  }
  ALSoundSource* src = GetSource(id);
  if (src->GetState() != ALSoundSource::SoundState::PAUSED)
  {
    alSourceRewind(src->Id);
  }
  alSourcePlay(src->Id);
  return alGetError() == AL_NO_ERROR;
}

bool ALDevice::Stop(size_t id)
{
  if (!id)
  {
    return false;
  }
  ALSoundSource* src = GetSource(id);
  alSourceStop(src->Id);
  return alGetError() == AL_NO_ERROR;
}

bool ALDevice::Pause(size_t id)
{
  if (!id)
  {
    return false;
  }
  ALSoundSource* src = GetSource(id);
  alSourcePause(src->Id);
  return alGetError() == AL_NO_ERROR;
}

ALSoundSource* ALDevice::GetSource(size_t id)
{
  if (!id)
  {
    return nullptr;
  }
  std::lock_guard<std::mutex> l(SoundsMutex);
  return Sounds[id - 1];
}

std::vector<ALSoundSource*> ALDevice::GetSources()
{
  std::lock_guard<std::mutex> l(SoundsMutex);
  return Sounds;
}

ALSoundSource::~ALSoundSource()
{
  if (Id)
  {
    alDeleteSources(1, &Id);
  }
  if (BufferId)
  {
    alDeleteBuffers(1, &BufferId);
  }
  free(PcmData);
}

size_t ALSoundSource::ReadOgg(void* ptr, uint32 size)
{
  size_t sizeToRead = std::min<size_t>(size, Sound->GetResourceSize() - DataOffset);
  uint8* src = (uint8*)Sound->GetResourceData() + DataOffset;
  std::memcpy(ptr, src, sizeToRead);
  DataOffset += sizeToRead;
  return sizeToRead;
}

int32 ALSoundSource::SeekOgg(uint64 offset, int whence)
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
    DataOffset = Sound->GetResourceSize() - offset;
    break;
  }
  return DataOffset;
}

uint32 ALSoundSource::TellOgg()
{
  return DataOffset;
}
