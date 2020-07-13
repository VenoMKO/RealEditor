#pragma once
#include "Core.h"
#include <fstream>
#include <functional>
#include <algorithm>

// Abstract stream to read and write data
class FStream {
public:
  FStream()
  {}

  virtual ~FStream()
  {}

  inline FStream& operator<<(int8& v)
  {
    SerializeBytes(&v, 1);
    return *this;
  }

  inline FStream& operator<<(uint8& v)
  {
    SerializeBytes(&v, 1);
    return *this;
  }

  inline FStream& operator<<(int16& v)
  {
    SerializeBytes(&v, 2);
    return *this;
  }

  inline FStream& operator<<(uint16& v)
  {
    SerializeBytes(&v, 2);
    return *this;
  }

  inline FStream& operator<<(int32& v)
  {
    SerializeBytes(&v, 4);
    return *this;
  }

  inline FStream& operator<<(uint32& v)
  {
    SerializeBytes(&v, 4);
    return *this;
  }

  inline FStream& operator<<(int64& v)
  {
    SerializeBytes(&v, 8);
    return *this;
  }

  inline FStream& operator<<(uint64& v)
  {
    SerializeBytes(&v, 8);
    return *this;
  }

  inline FStream& operator<<(char& v)
  {
    SerializeBytes(&v, 1);
    return *this;
  }

  inline FStream& operator<<(wchar& v)
  {
    SerializeBytes(&v, 2);
    return *this;
  }

  inline FStream& operator<<(bool& v)
  {
    int32 intv = v ? 1 : 0;
    SerializeBytes(&intv, 4);
    return *this;
  }

  inline FStream& operator<<(float& v)
  {
    SerializeBytes(&v, 4);
    return *this;
  }

  FStream& operator<<(std::string& s);

  template <typename T>
  inline FStream& operator<<(std::vector<T>& arr)
  {
    uint32 cnt = (uint32)arr.size();
    (*this) << cnt;
    if (Reading)
    {
      arr.clear();
      for (uint32 idx = 0; idx < cnt; ++idx)
      {
        T& item = arr.emplace_back(T());
        (*this) << item;
      }
    }
    else
    {
      for (T& item : arr)
      {
        (*this) << item;
      }
    }
    return *this;
  }

  template <typename Tk, typename Tv>
  inline FStream& operator<<(std::map<Tk, Tv>& map)
  {
    uint32 cnt = (uint32)map.size();
    (*this) << cnt;
    if (Reading)
    {
      map.clear();
      for (uint32 idx = 0; idx < cnt; ++idx)
      {
        Tk k; Tv v;
        (*this) << k;
        (*this) << v;
        map.emplace(k, v);
      }
    }
    else
    {
      for (std::pair<const Tk, Tv>& pair : map)
      {
        Tk k(pair.first);
        (*this) << k;
        (*this) << pair.second;
      }
    }
    return *this;
  }

  virtual void SerializeBytes(void* ptr, FILE_OFFSET size) = 0;

  virtual void SetPosition(FILE_OFFSET offset) = 0;

  virtual FILE_OFFSET GetPosition() = 0;

  virtual FILE_OFFSET GetSize() = 0;

  inline bool IsReading() const
  {
    return Reading;
  }

  virtual bool IsGood() const = 0;

  virtual void Close() = 0;

  inline void SetPackage(FPackage* p)
  {
    Package = p;
  }

  inline FPackage* GetPackage() const
  {
    return Package;
  }

protected:
  bool Reading = false;
  FPackage* Package = nullptr;
};

class FReadStream : public FStream {
public:
  FReadStream(const std::string& path)
    : FStream()
    , Path(path)
    , Stream(A2W(path), std::ios::binary)
  {
    Reading = true;
    bool test = Stream.good();
    int k = 1;
  }

  FReadStream(const FReadStream& s)
    : FStream()
    , Path(s.Path)
    , Stream(A2W(s.Path), std::ios::in | std::ios::binary)
  {
    Reading = true;
  }

  FReadStream(FReadStream* s)
    : FStream()
    , Path(s->Path)
    , Stream(A2W(s->Path), std::ios::in | std::ios::binary)
  {
    Reading = true;
    Stream.seekg(s->Stream.tellg());
  }

  ~FReadStream()
  {
    Stream.close();
  }

  void SerializeBytes(void* ptr, FILE_OFFSET size) override
  {
    if (size)
    {
      Stream.read((char*)ptr, size);
    }
  }

  FILE_OFFSET GetPosition() override
  {
    return (FILE_OFFSET)Stream.tellg();
  }

  void SetPosition(FILE_OFFSET pos) override
  {
    Stream.seekg(pos);
  }

  FILE_OFFSET GetSize() override
  {
    FILE_OFFSET size = 0;
    if (IsGood())
    {
      FILE_OFFSET tmpPos = GetPosition();
      Stream.seekg(0, std::ios::end);
      size = GetPosition();
      SetPosition(tmpPos);
    }
    return size;
  }

  bool IsGood() const override
  {
    return Stream.good();
  }

  void Close() override
  {
    Stream.close();
  }

protected:
  std::string Path;
  std::ifstream Stream;
};

class FWriteStream : public FStream {
public:
  FWriteStream(const std::string& path, bool trunk = true)
    : FStream()
    , Stream(A2W(path), trunk ? (std::ios::out | std::ios::trunc | std::ios::binary) : (std::ios::out | std::ios::binary))
  {
    Reading = false;
  }

  ~FWriteStream()
  {
    Stream.close();
  }

  void SerializeBytes(void* ptr, FILE_OFFSET size) override
  {
    if (size)
    {
      Stream.write((const char*)ptr, size);
    }
  }

  FILE_OFFSET GetPosition() override
  {
    return (FILE_OFFSET)Stream.tellp();
  }

  void SetPosition(FILE_OFFSET pos) override
  {
    Stream.seekp(pos);
  }

  FILE_OFFSET GetSize() override
  {
    FILE_OFFSET size = 0;
    if (IsGood())
    {
      FILE_OFFSET tmpPos = GetPosition();
      Stream.seekp(0, std::ios::end);
      size = GetPosition();
      SetPosition(tmpPos);
    }
    return size;
  }

  bool IsGood() const override
  {
    return Stream.good();
  }

  void Close() override
  {
    Stream.close();
  }

protected:
  std::string Path;
  std::ofstream Stream;
};