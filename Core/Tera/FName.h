#pragma once
#include "Core.h"
#include "FString.h"

#define MAX_NAME_ENTRY 1024

class FNameEntry {
public:
  FNameEntry() = default;

  FNameEntry(const FString& value)
    : String(value)
  {
    DBreakIf(String.Length() > MAX_NAME_ENTRY);
#if TERMINATE_NEW_NAMES
    if (String.Back())
    {
      String += '\0';
    }
#endif
  }

  FNameEntry(const char* value)
    : String(value)
  {
    DBreakIf(String.Length() > MAX_NAME_ENTRY);
#if TERMINATE_NEW_NAMES
    if (String.Back())
    {
      String += '\0';
    }
#endif
  }

  uint64 GetFlags() const
  {
    return Flags;
  }

  const FString& GetString() const
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
    DBreakIf(String.Length() > MAX_NAME_ENTRY);
#if TERMINATE_EXISTING_NAMES
    if (String.Back())
    {
      String += '\0';
    }
#endif
  }

  bool operator==(const FString& name) const;
  bool operator==(const char* name) const;
  bool operator<(const FNameEntry& b) const;

  friend FStream& operator<<(FStream& s, FNameEntry& e);

private:
  FString String;
  uint64 Flags = (RF_TagExp | RF_LoadForClient | RF_LoadForServer | RF_LoadForEdit);
};

class FName {
public:
  friend class FStream;
  FName() = default;

  FName(FPackage* package)
    : Package(package)
  {}

  // TODO: fixme
  FName(const char* string)
  {
    DBreakIf(strcmp(string, NAME_None));
  }

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

  // Get FString value
  virtual FString String(bool number = USE_FNAME_NUMBERS) const;
  // Get FString value
  virtual void GetString(FString& output, bool number = USE_FNAME_NUMBERS) const;
  // Does NOT include numbers regardless of the USE_FNAME_NUMBERS!!!
  virtual const FString& GetString() const;
  
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

#ifdef _DEBUG
  void SetDebugValue(FString& s)
  {
    Value = s.String();
  }
#endif

protected:
  NAME_INDEX Index = INDEX_NONE;
  int32 Number = 0;
  FPackage* Package = nullptr;
#ifdef _DEBUG
  std::string Value;
#endif
};

class VName : public FName {
public:
  VName() = default;
  VName(const VName& name);
  VName(const FString& name, int32 number = 0);
  VName(const char* name, int32 number = 0);


  FString String(bool number = USE_FNAME_NUMBERS) const override;
  void GetString(FString& output, bool number = USE_FNAME_NUMBERS) const override;
};