#include "FStream.h"

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
