#pragma once
#include "Core.h"

class FNameEntry {
public:

  uint64 GetFlags() const
  {
    return Flags;
  }

  std::string GetString() const
  {
    return String;
  }

  void SetString(const std::string& string)
  {
    String = string;
  }

  friend FStream& operator<<(FStream& s, FNameEntry& e);

private:
  std::string String;
  uint64 Flags = 0;
};

class FName {
public:
  FName()
  {}

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
  bool operator==(const std::string& s) const;
  bool operator<(const FName& n) const;

  friend FStream& operator<<(FStream& s, FName& n);

  std::string String() const;
private:
  NAME_INDEX Index = 0;
  int32 Number = 0;
  
  FPackage* Package = nullptr;
#ifdef _DEBUG
  std::string Value;
#endif
};