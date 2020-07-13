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
  return Wstricmp(String(), n.String());
}

bool FName::operator==(const std::string& s) const
{
  return Wstricmp(String(), s);
}

bool FName::operator<(const FName& n) const
{
  std::wstring a = A2W(String());
  std::wstring b = A2W(n.String());
  return a < b;
}

std::string FName::String() const
{
  std::string value = Package->GetIndexedName(Index);
  if (Number)
  {
    value += "_" + std::to_string(Number);
  }
  return value;
}

FStream& operator<<(FStream& s, FName& n)
{
  SET_PACKAGE(s, n);
  s << n.Index;
  s << n.Number;
#ifdef _DEBUG
  n.Value = n.String();
#endif
  return s;
}