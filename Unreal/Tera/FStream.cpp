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

FStream& FStream::operator<<(std::string& s)
{
  if (IsReading())
  {
    s.resize(0);
    int32 len = 0;
    (*this) << len;
    if (len > 0)
    {
      const int32 tlen = len - 1;
      s.resize(tlen);
      char* data = (char*)malloc(len);
      SerializeBytes(data, len);
      memcpy_s(&s[0], tlen, data, tlen);
      free(data);
    }
    else if (len < 0)
    {
      len = -len;
      const int32 tlen = len - 1;
      wchar* data = (wchar*)malloc((size_t)len * 2);
      SerializeBytes(data, len * 2);
      std::string tmp = W2A(data, tlen);
      s.resize(tmp.size());
      DBreakIf(tmp.size() != s.size());
      memcpy_s(&s[0], s.size(), &tmp[0], tmp.size());
      free(data);
    }
  }
  else
  {
    if (s.empty())
    {
      int32 len = 0;
      (*this) << len;
      return *this;
    }
    if (IsAnsi(s))
    {
      int32 len = (int32)s.size() + 1;
      (*this) << len;
      SerializeBytes(&s[0], len - 1);
      char term = '\0';
      (*this) << term;
    }
    else
    {
      std::wstring tmp = A2W(s);
      int32 len = (-(int32)s.size() - 1) * 2;
      (*this) << len;
      SerializeBytes(&tmp[0], len - 2);
      wchar term = '\0';
      (*this) << term;
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
