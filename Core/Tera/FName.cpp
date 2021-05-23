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

FString FName::String(bool number) const
{
  FString value;
  GetString(value, number);
  return value;
}

void FName::GetString(FString& str, bool number) const
{
  if (Index == INDEX_NONE)
  {
    str = NAME_None;
  }
  else
  {
    Package->GetIndexedName(Index, str);
    if (Number && number)
    {
      str += "_" + std::to_string(Number - 1);
    }
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