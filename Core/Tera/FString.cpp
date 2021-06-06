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

int32 Ch2Int(char input)
{
  if (input >= '0' && input <= '9')
    return input - '0';
  if (input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if (input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  return 0;
}

void FString::StringToBytes(const char* src, size_t len, unsigned char* dst)
{
  const char* srcPtr = src;
  for (int idx = 0; idx < len; idx+=2)
  {
    *(dst++) = Ch2Int(srcPtr[idx]) * 16 + Ch2Int(srcPtr[idx + 1]);
  }
}
