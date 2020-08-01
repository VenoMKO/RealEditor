#include "FStream.h"
#include "FPackage.h"

FStream& FStream::operator<<(FString& s)
{
  if (IsReading())
  {
    s.Resize(0);
    int32 len = 0;
    (*this) << len;
    if (len > 0)
    {
      s.Resize(len);
      SerializeBytes(&s[0], len);
    }
    else if (len < 0)
    {
      len = -len * 2;
      wchar* data = (wchar*)malloc(len);
      SerializeBytes(data, len);
      s += data;
      free(data);
    }
  }
  else
  {
    int32 len = (int32)s.Size();
    if (!s.IsAnsi())
    {
      std::wstring wstr = s;
      len = -(int32)wstr.size();
      (*this) << len;
      SerializeBytes((void*)s.C_str(), len * -2);
    }
    else
    {
      (*this) << len;
      SerializeBytes((void*)s.C_str(), len);
    }
  }
  return *this;
}

uint16 FStream::GetFV() const
{
  return Package ? Package->GetFileVersion() : 0;
}

uint16 FStream::GetLV() const
{
  return Package ? Package->GetLicenseeVersion() : 0;
}
