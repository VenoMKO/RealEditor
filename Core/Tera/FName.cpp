#include "FName.h"
#include "FPackage.h"
#include "FStream.h"

FStream& operator<<(FStream& s, FNameEntry& e)
{
  s << e.String;
  s << e.Flags;
  return s;
}

bool FName::operator==(const FName& n) const
{
  return String().ToUpper() == n.String().ToUpper();
}

bool FName::operator==(const FString& s) const
{
  return String().ToUpper() == s.ToUpper();
}

bool FName::operator==(const char* s) const
{
  return String() == s;
}

bool FName::operator!=(const FName& n) const
{
  return String().ToUpper() != n.String().ToUpper();
}

bool FName::operator!=(const FString& s) const
{
  return String().ToUpper() != s.ToUpper();
}

bool FName::operator!=(const char* s) const
{
  return String() != s;
}

bool FName::operator<(const FName& n) const
{
  return String() < n.String();
}

FString FName::String() const
{
  FString value;
  GetString(value);
  return value;
}

void FName::GetString(FString& str) const
{
  Package->GetIndexedName(Index, str);
  if (Number)
  {
    str += "_" + std::to_string(Number);
  }
}

void FName::SetString(const FString& str)
{
  Index = Package->GetNameIndex(str, true);
  Number = 0;
#ifdef _DEBUG
  Value = String();
#endif
}

FStream& operator<<(FStream& s, FName& n)
{
  if (s.IsReading() && s.GetPackage())
  {
    n.Package = s.GetPackage();
  }
  // Check if name's package does not match stream's one. Thus the Index is invalid.
  DBreakIf(!s.IsReading() && s.GetPackage() != n.Package);
  s << n.Index;
  s << n.Number;
#ifdef _DEBUG
  n.Value = n.String();
#endif
  return s;
}