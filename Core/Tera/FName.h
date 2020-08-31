#pragma once
#include "Core.h"
#include "FString.h"

#define MAX_NAME_ENTRY 1024

class FNameEntry {
public:
  FNameEntry()
  {}

  FNameEntry(const FString& value)
    : String(value)
  {}

  uint64 GetFlags() const
  {
    return Flags;
  }

  FString GetString() const
  {
    return String;
  }

  void GetString(FString& output) const
  {
    output += String;
  }

  void SetString(const FString& string)
  {
    String = string;
  }

  friend FStream& operator<<(FStream& s, FNameEntry& e);

private:
  FString String;
  uint64 Flags = 0;
};

class FName {
public:
  FName()
  {}

  FName(FPackage* package)
    : Package(package)
  {}

  FName(FPackage* package, const FString& value)
    : Package(package)
  {
    SetString(value);
  }

  FName(const FName& n)
    : Index(n.Index)
    , Number(n.Number)
    , Package(n.Package)
  {
#ifdef _DEBUG
    Value = n.Value;
#endif
  }

  bool operator==(const FName& n) const;
  bool operator==(const FString& s) const;
  bool operator==(const char* s) const;
  bool operator!=(const FName& n) const;
  bool operator!=(const FString& s) const;
  bool operator!=(const char* s) const;
  bool operator<(const FName& n) const;

  friend FStream& operator<<(FStream& s, FName& n);

  FString String() const;
  void GetString(FString& output) const;
  void SetString(const FString& str);

  void SetIndex(NAME_INDEX index)
  {
    Index = index;
  }

  void SetNumber(int32 number)
  {
    Number = number;
  }

  NAME_INDEX GetIndex() const
  {
    return Index;
  }

  int32 GetNumber() const
  {
    return Number;
  }

  FPackage* GetPackage() const
  {
    return Package;
  }

  void SetPackage(FPackage* package)
  {
    Package = package;
  }

private:
  NAME_INDEX Index = 0;
  int32 Number = 0;
  
  FPackage* Package = nullptr;
#ifdef _DEBUG
  std::string Value;
#endif
};