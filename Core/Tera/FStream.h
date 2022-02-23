#pragma once
#include "Core.h"
#include "FString.h"

#include <fstream>
#include <functional>
#include <algorithm>

#define IF_BUFFER_SIZE 1024 * 1024

// Abstract stream to read and write data
class FStream {
public:
  FStream() = default;

  virtual ~FStream() = default;

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
    v = (bool)intv;
    return *this;
  }

  inline FStream& operator<<(float& v)
  {
    SerializeBytes(&v, 4);
    return *this;
  }

  inline FStream& operator<<(double& v)
  {
    SerializeBytes(&v, 8);
    return *this;
  }

  FStream& operator<<(class FName& n);

  FStream& operator<<(FString& s);
  FStream& operator<<(FStringRef& r);

  template <typename T>
  inline FStream& operator<<(std::vector<T>& arr)
  {
    uint32 cnt = (uint32)arr.size();
    (*this) << cnt;
    if (Reading)
    {
      arr.clear();
      arr.reserve(cnt);
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

  template <typename Tk, typename Tv>
  inline FStream& operator<<(std::multimap<Tk, Tv>& map)
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

  template <typename Tk, typename Tv>
  inline FStream& operator<<(std::unordered_map<Tk, Tv>& map)
  {
    uint32 cnt = (uint32)map.size();
    (*this) << cnt;
    if (Reading)
    {
      map.clear();
      map.reserve(cnt);
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

  virtual void SerializeObjectRef(void*& obj, PACKAGE_INDEX& index);

  void SerializeCompressed(void* v, int32 length, ECompressionFlags flags, bool concurrent = true);
  
  template <typename T>
  void SerializeUntypeBulkDataArray(std::vector<T*>& v, UObject* owner)
  {
    int32 cnt = (int32)v.size();
    *this << cnt;
    if (IsReading())
    {
      for (int32 idx = 0; idx < cnt; ++idx)
      {
        T* e = new T();
        e->Serialize(*this, owner, idx);
        v.push_back(e);
      }
    }
    else
    {
      for (int32 idx = 0; idx < cnt; ++idx)
      {
        T* e = v[idx];
        e->Serialize(*this, owner, idx);
      }
    }
  }

  virtual void SerializeBytes(void* ptr, FILE_OFFSET size) = 0;
  virtual void SerializeBytesAt(void* ptr, FILE_OFFSET offset, FILE_OFFSET size) = 0;

  virtual void SetPosition(FILE_OFFSET offset) = 0;

  virtual FILE_OFFSET GetPosition() = 0;

  virtual FILE_OFFSET GetSize() = 0;

  inline bool IsReading() const
  {
    return Reading;
  }

  virtual FStream& SerializeObjectArray(std::vector<class UObject*>& objects, std::vector<PACKAGE_INDEX>& indices);

  virtual bool IsGood() const = 0;

  virtual void Close() = 0;

  inline void SetPackage(FPackage* p)
  {
    Package = p;
  }

  virtual FPackage* GetPackage() const
  {
    return Package;
  }

  // false = dont call Load() on objects serialized via operator '<<'
  inline bool GetLoadSerializedObjects() const
  {
    return LoadSerializedObjects;
  }

  inline void SetLoadSerializedObjects(bool flag)
  {
    LoadSerializedObjects = flag;
  }

  virtual FStream& SerializeNameIndex(class FName& n);

  virtual uint16 GetFV() const;

  virtual uint16 GetLV() const;

protected:
  bool Reading = false;
  FPackage* Package = nullptr;
  bool LoadSerializedObjects = true;
};

class FReadStream : public FStream {
public:
  FReadStream(const std::string& path)
    : FStream()
    , Path(path)
  {
    InitStream(Path.WString());
    Reading = true;
  }

  FReadStream(const std::wstring& path)
    : FStream()
    , Path(W2A(path))
  {
    InitStream(Path.WString());
    Reading = true;
  }

  FReadStream(const FString& path)
    : FStream()
    , Path(path)
  {
    InitStream(Path.WString());
    Reading = true;
  }

  FReadStream(const FReadStream& s)
    : FStream()
    , Path(s.Path)
  {
    InitStream(Path.WString());
    Reading = true;
  }

  FReadStream(FReadStream* s)
    : FStream()
    , Path(s->Path)
  {
    InitStream(Path.WString());
    Reading = true;
    Stream.seekg(s->Stream.tellg());
  }

  ~FReadStream()
  {
    free(IfBuffer);
    Stream.close();
  }

  void SerializeBytes(void* ptr, FILE_OFFSET size) override
  {
    if (ptr && size)
    {
      Stream.read((char*)ptr, size);
      DBreakIf(!IsGood());
    }
  }

  void SerializeBytesAt(void* ptr, FILE_OFFSET offset, FILE_OFFSET size) override
  {
    auto pos = Stream.tellg();
    Stream.seekg(offset);
    Stream.read((char*)ptr, size);
    Stream.seekg(pos);
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
  void InitStream(const std::wstring& path)
  {
    InitBuffer(IF_BUFFER_SIZE);
    Stream.rdbuf()->pubsetbuf(IfBuffer, IF_BUFFER_SIZE);
    Stream.open(path, std::ios::in | std::ios::binary);
  }

  void InitBuffer(size_t size)
  {
    if (IfBuffer)
    {
      free(IfBuffer);
    }
    IfBuffer = (char*)malloc(size);
  }

protected:
  FString Path;
  std::ifstream Stream;
  char* IfBuffer = nullptr;
};

class FWriteStream : public FStream {
public:
  FWriteStream(const FString& path, bool trunk = true)
    : FStream()
    , Path(path)
    , Stream(path.WString(), trunk ? (std::ios::out | std::ios::trunc | std::ios::binary) : (std::ios::out | std::ios::binary))
  {
    Reading = false;
  }

  FWriteStream(const std::wstring& path, bool trunk = true)
    : FStream()
    , Path(W2A(path))
    , Stream(path, trunk ? (std::ios::out | std::ios::trunc | std::ios::binary) : (std::ios::out | std::ios::binary))
  {
    Reading = false;
  }

  ~FWriteStream()
  {
    Stream.close();
  }

  void SerializeBytes(void* ptr, FILE_OFFSET size) override
  {
    if (ptr && size)
    {
      Stream.write((const char*)ptr, size);
    }
  }

  void SerializeBytesAt(void* ptr, FILE_OFFSET offset, FILE_OFFSET size) override
  {
    auto pos = Stream.tellp();
    Stream.seekp(offset);
    Stream.write((char*)ptr, size);
    Stream.seekp(pos);
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

  void Flush()
  {
    Stream.flush();
  }

protected:
  FString Path;
  std::ofstream Stream;
};

// Stream to read from memory. fakeOffset emulates position in file for Get/SetPosition
class MReadStream : public FStream {
public:
  MReadStream(void* data, bool takerOwnership, size_t size, size_t fakeOffset = 0)
    : OwnesMemory(takerOwnership)
    , Data((uint8*)data)
    , Offset(fakeOffset)
    , Size(size)
  {
    Reading = true;
    Good = size;
    Position = Offset;
  }

  ~MReadStream()
  {
    if (Data && OwnesMemory)
    {
      free(Data);
    }
  }

  void SerializeBytes(void* ptr, FILE_OFFSET size) override
  {
    if (!ptr || !size)
    {
      return;
    }
    if (!Good || Position + size > Offset + Size)
    {
      DBreak();
      Good = false;
      return;
    }
    memcpy(ptr, Data + Position, size);
    Position += size;
  }

  void SerializeBytesAt(void* ptr, FILE_OFFSET offset, FILE_OFFSET size) override
  {
    if (!Good || offset < Offset || (Offset && Offset > offset) || offset + size > Offset + Size)
    {
      DBreak();
      Good = false;
      return;
    }
    memcpy(ptr, Data + Offset + offset, size);
  }

  void SetPosition(FILE_OFFSET offset) override
  {
    if (!Good || (Offset && offset < Offset) || (offset - Offset) > Offset + Size)
    {
      DBreak();
      Good = false;
      return;
    }
    Position = (offset - Offset);
  }

  FILE_OFFSET GetPosition() override
  {
    return (FILE_OFFSET)(Position + Offset);
  }

  FILE_OFFSET GetSize() override
  {
    return (FILE_OFFSET)Size;
  }

  bool IsGood() const override
  {
    return Good;
  }

  void Close() override
  {
    Good = false;
  }

  void* GetAllocation()
  {
    return Data;
  }

protected:
  bool Good = false;
  bool OwnesMemory = false;
  uint8* Data = nullptr;
  size_t Position = 0;
  size_t Offset = 0;
  size_t Size = 0;
};

class MWriteStream : public FStream {
public:
  MWriteStream(void* data, size_t size, size_t fakeOffset = 0)
    : Offset(fakeOffset)
    , Size(size)
  {
    if (data && size)
    {
      Data = (uint8*)malloc(size);
      memcpy_s(Data, size, data, size);
    }
    else if (size)
    {
      Data = (uint8*)malloc(size);
    }
    Reading = false;
    Good = true;
  }

  ~MWriteStream()
  {
    if (Data)
    {
      delete Data;
    }
  }

  void SerializeBytes(void* ptr, FILE_OFFSET size) override
  {
    if (!ptr || !size || !Good)
    {
      return;
    }
    if (Position + size > Size)
    {
      size_t newSize = Size + (Position + size) - Size;
      if (Data)
      {
        if (void* newData = realloc(Data, newSize))
        {
          Data = (uint8*)newData;
        }
        else
        {
          if (Data)
          {
            free(Data);
            Data = nullptr;
          }
          Good = false;
          return;
        }
      }
      else
      {
        Data = (uint8*)malloc(newSize);
      }
      Size = newSize;
    }
    memcpy(Data + Position, ptr, size);
    Position += size;
  }

  void SerializeBytesAt(void* ptr, FILE_OFFSET offset, FILE_OFFSET size) override
  {
  }

  void SetPosition(FILE_OFFSET offset) override
  {
    offset -= (FILE_OFFSET)Offset;
    Position = offset > Size ? Size : offset < 0 ? 0 : offset;
  }

  FILE_OFFSET GetPosition() override
  {
    return (FILE_OFFSET)(Position + Offset);
  }

  FILE_OFFSET GetSize() override
  {
    return (FILE_OFFSET)(Size + Offset);
  }

  bool IsGood() const override
  {
    return Good;
  }

  void Close() override
  {
    Good = false;
  }

  void* GetAllocation()
  {
    return Data;
  }

  FILE_OFFSET GetOffset() const
  {
    return (FILE_OFFSET)Offset;
  }

protected:
  bool Good = false;
  uint8* Data = nullptr;
  size_t Position = 0;
  size_t Offset = 0;
  size_t Size = 0;
};

// Memory stream for object transactions(i.e. Copy-Paste)
class MTransStream : public FStream {
public:
  MTransStream()
  {
    LoadSerializedObjects = false;
  }

  ~MTransStream()
  {
    if (Data)
    {
      free(Data);
    }
  }

  // FStream interface

  void SerializeBytes(void* ptr, FILE_OFFSET size) override;

  void SerializeBytesAt(void* ptr, FILE_OFFSET offset, FILE_OFFSET size) override
  {}

  void SerializeObjectRef(void*& obj, PACKAGE_INDEX& index) override;

  FStream& SerializeNameIndex(class FName& n) override;

  void SetPosition(FILE_OFFSET offset) override
  {
    Position = offset > DataSize ? DataSize : offset < 0 ? 0 : offset;
  }

  FILE_OFFSET GetPosition() override
  {
    return (FILE_OFFSET)(Position);
  }

  FILE_OFFSET GetSize() override
  {
    return (FILE_OFFSET)(DataSize);
  }

  bool IsGood() const override
  {
    return Good;
  }

  void Close() override
  {}

  uint16 GetFV() const override;

  uint16 GetLV() const override;

  FPackage* GetPackage() const override
  {
    return Reading ? DestPackage : SourcePackage.get();
  }

  // Transaction methods

  // Set stream direction: true - copy to the stream, false - paste from the stream
  void SetIsReading(bool flag)
  {
    DBreakIf(flag && !DestPackage);
    Reading = flag;
  }

  void SetSourcePackage(std::shared_ptr<FPackage> package);

  void SetDestinationPackage(FPackage* package);

  bool IsEmpty() const
  {
    return Depends.empty();
  }

  void Clear();

  void ClearData();

  // Get objects with no outer
  std::map<UObject*, UObject*> GetOrphanObjects();

  // Update orphans map
  void ApplyOrphansMap(const std::map<UObject*, UObject*>& map);

  // Copy object to the stream
  bool StartObjectTransaction(class UObject* obj);

  // Paste stream contents to the object. If object is a UPackage the transacting object will placed inside of it
  bool EndObjectTransaction(class UObject* obj);

  // Get the source object
  UObject* GetTransactingObject() const
  {
    return Root;
  }

  // Get the copied object
  UObject* GetTransactedObject() const;

  // Set custom root object name
  void SetNewObjectName(const FString& name)
  {
    CustomRootName = name;
  }

  FString GetError() const
  {
    return ErrorText;
  }

protected:
  // Save the obj to the stream. Has nothing to do with the SerializeObjectRef
  bool SerializeObject(class UObject* obj, bool recursiv);
  // Restore stream storage to the obj
  bool DeserializeObject(class UObject* obj);

  UObject* FindDestinationObject(class UObject* obj) const;

protected:
  bool Good = true;
  FString ErrorText;
  
  uint8_t* Data = nullptr;
  size_t Position = 0;
  size_t DataSize = 0;
  size_t AllocationSize = 0;
  size_t BufferSize = 1024 * 1024;

  std::shared_ptr<FPackage> SourcePackage = nullptr;
  FPackage* DestPackage = nullptr;

  UObject* Root = nullptr;
  FString CustomRootName;

  std::vector<FString> Names;
  std::vector<UObject*> Depends;

  std::vector<FObjectImport*> Imports;
  std::vector<FObjectExport*> Unresolved;

  std::map<UObject*, UObject*> OrphansMap;
  std::map<UObject*, UObject*> DependsMap;
  std::map<FObjectImport*, FObjectImport*> ImportsMap;

  std::map<PACKAGE_INDEX, FObjectResource*> UnresolvedRemap;
};