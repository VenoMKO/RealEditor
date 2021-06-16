#pragma once
#include <Tera/Core.h>
#include <Tera/FStructs.h>

class USoundNodeWave;

class SoundTravaller {
public:

  ~SoundTravaller()
  {
    free(Data);
  }

  std::string GetError() const
  {
    return Error;
  }

  void SetData(void* data, FILE_OFFSET size)
  {
    free(Data);
    Data = data;
    DataSize = size;
  }

  bool Visit(USoundNodeWave* wave);

  size_t ReadOgg(void* ptr, uint32 size);
  int32 SeekOgg(uint64 offset, int whence);
  uint32 TellOgg();

private:
  struct FSoundInfo
  {
    uint32 NumChannels = 0;
    uint32 SampleRate = 0;
    float Duration = 0;
  };

  bool GetOggInfo(FSoundInfo& info);

private:
  std::string Error;
  void* Data = nullptr;
  FILE_OFFSET DataSize = 0;
  FILE_OFFSET DataOffset = 0;
};