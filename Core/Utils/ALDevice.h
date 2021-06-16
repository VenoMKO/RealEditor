#pragma once
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <Tera/Core.h>

class ALSoundObserver {
public:
  virtual ~ALSoundObserver() = default;

  virtual void OnSoundStarted(size_t source) = 0;
  virtual void OnSoundPaused(size_t source) = 0;
  virtual void OnSoundStopped(size_t source) = 0;
};

struct ALSoundSource {
  enum class SoundState {
    STOPPED,
    PLAYING,
    PAUSED,
  };

  ~ALSoundSource();

  void SetState(SoundState state)
  {
    std::scoped_lock<std::mutex> l(StateMutex);
    State = state;
  }

  SoundState GetState() const
  {
    std::scoped_lock<std::mutex> l(StateMutex);
    return State;
  }

  class USoundNodeWave* Sound = nullptr;
  class ALSoundObserver* Observer = nullptr;
  uint32 Id = 0;
  uint32 BufferId = 0;
  mutable std::mutex StateMutex;
  SoundState State = SoundState::STOPPED;

  size_t ReadOgg(void* ptr, uint32 size);
  int32 SeekOgg(uint64 offset, int whence);
  uint32 TellOgg();

  FILE_OFFSET DataOffset = 0;
  void* PcmData = nullptr;
};

class ALDevice {
public:
  // The ctor throws on error!
  ALDevice(const char* deviceName = nullptr);
  ~ALDevice();

  size_t AddSoundSource(ALSoundObserver* observer, class USoundNodeWave* sound);
  void RemoveSoundSource(size_t id);
  ALSoundSource::SoundState GetSoundState(size_t id);

  bool Play(size_t id);
  bool Stop(size_t id);
  bool Pause(size_t id);

protected:
  ALSoundSource* GetSource(size_t id);
  std::vector<ALSoundSource*> GetSources();

protected:
  struct ALCdevice* Device = nullptr;
  struct ALCcontext* Context = nullptr;
  std::mutex SoundsMutex;
  std::vector<ALSoundSource*> Sounds;
  std::thread StatePoller;
  std::atomic_bool Alive;
};