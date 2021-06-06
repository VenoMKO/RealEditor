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

static const char HexLookupTable[] = "0123456789ABCDEF";

void FString::BytesToString(const char* src, size_t len, char* dst)
{
  const char* srcPtr = src;
  for (auto count = len; count > 0; --count)
  {
    unsigned char ch = *srcPtr++;
    *dst++ = HexLookupTable[ch >> 4];
    *dst++ = HexLookupTable[ch & 0x0f];
  }
}
