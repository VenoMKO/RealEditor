#include "FString.h"
#include "FStream.h"
#include "FPackage.h"

FString FStringRef::GetString()
{
  FReadStream s(Package->GetDataPath());
  s.SetPackage(Package);
  return GetString(s);
}

FString FStringRef::GetString(FStream& s)
{
  if (!Cached)
  {
    if (Size < 0 || Offset < 0)
    {
      UThrow("Invalid string offset!");
    }
    s.SetPosition(Offset);
    Cached = new FString();
    s << *Cached;
  }
  return *Cached;
}