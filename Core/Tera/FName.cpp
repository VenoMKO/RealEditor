#include "FName.h"
#include "FPackage.h"
#include "FStream.h"

std::vector<FNameEntry> GVirtualNames;

bool FNameEntry::operator==(const FString& name) const
{
  return String == name;
}

bool FNameEntry::operator==(const char* name) const
{
  return String == name;
}

bool FNameEntry::operator<(const FNameEntry& b) const
{
  return String < b.String;
}

FStream& operator<<(FStream& s, FNameEntry& e)
{
  s << e.String;
  s << e.Flags;
  return s;
}

bool FName::operator==(const FName& n) const
{
  if (Number != n.Number)
  {
    return false;
  }
  if (Package == n.Package)
  {
    return n.Index == Index;
  }
  return GetString() == n.GetString();
}

bool FName::operator==(const FString& s) const
{
  return String(true) == s;
}

bool FName::operator==(const char* s) const
{
  return String(true) == s;
}

bool FName::operator!=(const FName& n) const
{
  return !operator==(n);
}

bool FName::operator!=(const FString& s) const
{
  return !operator==(s);
}

bool FName::operator!=(const char* s) const
{
  return !operator==(s);
}

bool FName::operator<(const FName& n) const
{
  return String() < n.String();
}

FString FName::String(bool number) const
{
  FString value;
  GetString(value, number);
  return value;
}

void FName::GetString(FString& str, bool number) const
{
  if (Index < 0)
  {
    GVirtualNames[-Index - 1].GetString(str);
  }
  else
  {
    Package->GetIndexedNameString(Index, str);
  }
  if (Number && number)
  {
    str += "_" + std::to_string(Number - 1);
  }
}

const FString& FName::GetString() const
{
  if (Index < 0)
  {
    return GVirtualNames[-Index - 1].GetString();
  }
  return Package->GetIndexedNameEntry(Index).GetString();
}

void FName::SetString(const FString& str)
{
  Index = Package->GetNameIndex(str, true);
  Number = 0;
#ifdef _DEBUG
  Value = String();
#endif
}

VName::VName(const FString& name, int32 number)
{
  NAME_INDEX found = INDEX_NONE;
  for (NAME_INDEX idx = 0; idx < GVirtualNames.size(); ++idx)
  {
    if (GVirtualNames[idx] == name)
    {
      found = idx;
      break;
    }
  }
  if (found == INDEX_NONE)
  {
    found = (NAME_INDEX)GVirtualNames.size();
    GVirtualNames.emplace_back(name);
  }
  Index = found;
  Number = number;
}

VName::VName(const char* name, int32 number)
{
  NAME_INDEX found = INDEX_NONE;
  for (NAME_INDEX idx = 0; idx < GVirtualNames.size(); ++idx)
  {
    if (GVirtualNames[idx].GetString() == name)
    {
      found = -idx - 1;
      break;
    }
  }
  if (found == INDEX_NONE)
  {
    GVirtualNames.emplace_back(name);
    found = -(NAME_INDEX)GVirtualNames.size();
  }
  Index = found;
  Number = number;
}

VName::VName(const VName& name)
{
  Index = name.Index;
  Number = name.Number;
}

FString VName::String(bool number) const
{
  FString result;
  GetString(result, number);
  return result;
}

void VName::GetString(FString& output, bool number) const
{
  output = GVirtualNames[-Index - 1].GetString();
  if (number && Number)
  {
    output += "_" + std::to_string(Number);
  }
}
