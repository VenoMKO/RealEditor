#pragma once
#include <Tera/Core.h>
#include <Tera/FString.h>

class UTexture2D;
class TfcBuilder {
public:
  TfcBuilder(const FString& name)
    : Name(name)
  {}

  ~TfcBuilder()
  {
    free(TfcData);
  }

  inline void* GetAllocation(FILE_OFFSET& size) const
  {
    size = TfcDataSize;
    return TfcData;
  }

  inline FString GetError() const
  {
    return Error;
  }

  inline size_t GetCount() const
  {
    return Textures.size();
  }

  bool AddTexture(UTexture2D* texture);
  bool Compile();

private:
  FString Name;
  void* TfcData = nullptr;
  FILE_OFFSET TfcDataSize = 0;
  std::vector<UTexture2D*> Textures;
  FString Error;
};