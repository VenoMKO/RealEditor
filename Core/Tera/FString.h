#pragma once
#include <stdarg.h>
#include "Core.h"

#include <algorithm>

// Wrapper to keep track of '\0'
class FString {
public:
  static void BytesToString(const char* src, size_t len, char* dst);
  static void StringToBytes(const char* src, size_t len, unsigned char* dst);
  FString() = default;

  FString(const FString& str)
    : Data(str.Data)
  {}

  FString(const char* str)
    : Data(str)
  {}

  FString(const char* str, size_t len)
    : Data(str, len)
  {}

  FString(const wchar* str)
    : Data(W2A(str))
  {}

  FString(const std::string& str)
    : Data(str)
  {}

  FString(const std::wstring& str)
    : Data(W2A(str))
  {}

  inline size_t Size() const
  {
    return Data.size();
  }

  inline size_t Length() const
  {
    return Data.size();
  }

  inline bool Empty() const
  {
    return Data.empty();
  }

  inline const char* C_str() const
  {
    return Data.c_str();
  }

  inline const char& Front() const
  {
    return Data.front();
  }

  inline const char& Back() const
  {
    return Data.back();
  }

  inline auto Begin()
  {
    return Data.begin();
  }

  inline auto End()
  {
    return Data.end();
  }

  inline const auto Begin() const
  {
    return Data.begin();
  }

  inline const auto End() const
  {
    return Data.end();
  }

  inline void Clear()
  {
    Data.clear();
  }

  inline char& operator[](const size_t index)
  {
    return Data[index];
  }

  inline const char& operator[](const size_t index) const
  {
    return Data[index];
  }

  inline bool operator<(const FString& a) const
  {
    return Data < a.Data;
  }

  inline bool operator>(const FString& a) const
  {
    return Data > a.Data;
  }

  inline bool operator==(const char* a) const
  {
    if (!a && Data.empty())
    {
      return true;
    }
    size_t lenA = strlen(a);
    size_t lenB = Data.size();
    if (lenA && a[lenA - 1] == '\0')
    {
      lenA--;
    }
    if (lenB && Data[lenB - 1] == '\0')
    {
      lenB--;
    }
    if (lenA != lenB)
    {
      return false;
    }
    return memcmp(a, Data.c_str(), lenB) == 0;
  }

  inline std::wstring FStringByAppendingPath(const FString& path)
  {
    if (Empty())
    {
      return path.WString();
    }
    else if (path.Empty())
    {
      return WString();
    }

    if (Data.back() == '\\')
    {
      return A2W(Data + path.String());
    }
    return A2W(Data + '\\' + path.String());
  }

  inline std::wstring FilenameWString(bool extension = true) const
  {
    return A2W(FilenameString(extension));
  }

  inline std::string FilenameString(bool extension = true) const
  {
    if (extension)
    {
      size_t end = Data.size() - 1;
      if (Data[end] == '\0' || Data[end] == '\\')
      {
        end--;
      }
      if (Data[end] == '\\' || Data[end] == '\0')
      {
        end--;
      }
      size_t idx = Data.rfind('\\', end);
      if (idx != std::string::npos)
      {
        return Data.substr(idx + 1, end - idx + 1);
      }
    }
    else
    {
      size_t end = Data.size() - 1;
      if (Data[end] == '\0' || Data[end] == '\\')
      {
        end--;
      }
      if (Data[end] == '\\' || Data[end] == '\0')
      {
        end--;
      }
      size_t idx = Data.rfind('\\', end);
      if (idx != std::string::npos)
      {
        idx++;
        size_t dend = Data.find_last_of('.', end);
        if (dend != std::string::npos && dend > idx)
        {
          return Data.substr(idx, dend - idx);
        }
        return Data.substr(idx, end - idx + 1);
      }
    }
    return Data;
  }

  inline FString ToUpper() const
  {
    std::string result = Data;
    std::for_each(result.begin(), result.end(), [](char& c) {
      c = ::toupper(c);
    });
    return result;
  }

  inline bool Contains(const FString& a)
  {
    return Data.find(a.Data) != std::string::npos;
  }

  inline FString Filename(bool extension = true) const
  {
    return FString(FilenameString(extension));
  }

  inline FString FileExtension() const
  {
    auto pos = Data.find_last_of('.');
    if (pos != std::string::npos && pos < Data.size() - 1)
    {
      return Data.substr(pos + 1);
    }
    return FString();
  }

  inline bool EndsWith(const char* suffix) const
  {
    auto sfxlen = strlen(suffix);
    if (sfxlen > Data.size())
    {
      return false;
    }
    return Data.compare(Data.size() - sfxlen, sfxlen, suffix) == 0;
  }

  inline bool operator==(const FString& a) const
  {
    size_t lenA = a.Data.size();
    size_t lenB = Data.size();

    if (lenA && a.Data[lenA - 1] == '\0')
    {
      lenA--;
    }
    if (lenB && Data[lenB - 1] == '\0')
    {
      lenB--;
    }
    if (lenA != lenB)
    {
      return false;
    }
    return memcmp(a.Data.c_str(), Data.c_str(), lenB) == 0;
  }

  inline bool operator==(const std::string& a) const
  {
    size_t lenA = a.size();
    size_t lenB = Data.size();

    if (lenA && a[lenA - 1] == '\0')
    {
      lenA--;
    }
    if (lenB && Data[lenB - 1] == '\0')
    {
      lenB--;
    }
    if (lenA != lenB)
    {
      return false;
    }
    return memcmp(a.c_str(), Data.c_str(), lenB) == 0;
  }

  inline bool operator!=(const char* a) const
  {
    return !((*this) == a);
  }

  inline bool operator!=(const FString& a) const
  {
    return !((*this) == a);
  }

  inline bool operator!=(const std::string& a) const
  {
    return !((*this) == a);
  }

  inline operator std::string() const
  {
    return Data;
  }

  inline operator std::wstring() const
  {
    return A2W(Data);
  }

  inline void Erase(std::string::iterator& it)
  {
    Data.erase(it);
  }

  inline FString operator+(const std::string& str) const
  {
    FString s(*this);
    bool appnedNull = false;
    if (s.Size() && s.Back() == 0)
    {
      s.Data = s.Data.substr(0, s.Data.size() - 1);
      appnedNull = str.empty() || str.back() != 0;
    }
    s.Data += str;
    if (appnedNull)
    {
      s.Data.resize(s.Data.size() + 1);
    }
    return s;
  }

  inline FString& operator+=(const std::string& str)
  {
    bool appnedNull = false;
    if (Data.size() && Data.back() == 0)
    {
      Data = Data.substr(0, Data.size() - 1);
      appnedNull = str.empty() || str.back() != 0;
    }
    Data += str;
    if (appnedNull)
    {
      Data.resize(Data.size() + 1);
    }
    return *this;
  }

  inline FString& operator+=(const char& str)
  {
    bool appnedNull = false;
    if (Data.size() && Data.back() == 0)
    {
      Data = Data.substr(0, Data.size() - 1);
      appnedNull = str != 0;
    }
    Data += str;
    if (appnedNull)
    {
      Data.resize(Data.size() + 1);
    }
    return *this;
  }

  inline FString operator+(const FString& str) const
  {
    FString s(*this);
    bool appnedNull = false;
    if (s.Size() && s.Back() == 0)
    {
      s.Data = s.Data.substr(0, s.Data.size() - 1);
      appnedNull = str.Data.empty() || str.Data.back() != 0;
    }
    s.Data += str.Data;
    if (appnedNull)
    {
      s.Data.resize(s.Data.size() + 1);
    }
    return s;
  }

  inline FString& operator+=(const FString& str)
  {
    bool appnedNull = false;
    if (Data.size() && Data.back() == 0)
    {
      Data = Data.substr(0, Data.size() - 1);
      appnedNull = str.Data.empty() || str.Data.back() != 0;
    }
    Data += str.Data;
    if (appnedNull)
    {
      Data.resize(Data.size() + 1);
    }
    return *this;
  }

  inline FString operator+(const char* str) const
  {
    return (*this) + std::string(str);
  }

  inline FString& operator+=(const char* str)
  {
    return (*this) += std::string(str);
  }

  inline FString operator+(const wchar* str) const
  {
    return (*this) + W2A(std::wstring(str));
  }

  inline FString& operator+=(const wchar* str)
  {
    return (*this) += W2A(std::wstring(str));
  }

  inline std::string String() const
  {
    return Data;
  }

  inline std::wstring WString() const
  {
    return A2W(Data.c_str());
  }

  inline FString Substr(const size_t offset = 0, const size_t count = std::string::npos) const
  {
    return FString(Data.substr(offset, count));
  }

  inline size_t Find(const char ch, const size_t offset = 0) const
  {
    return Data.find(ch, offset);
  }

  inline size_t Find(const char* ptr, const size_t offset = 0) const
  {
    return Data.find(ptr, offset);
  }

  inline size_t Find(const char* ptr, const size_t offset, const size_t count) const
  {
    return Data.find(ptr, offset, count);
  }

  inline size_t Find(const FString& str, const size_t offset = 0) const
  {
    return Data.find(str.Data, offset);
  }

  inline void Resize(size_t size)
  {
    Data.resize(size);
  }

  inline void Reserve(size_t size)
  {
    Data.reserve(size);
  }

  void Terminate()
  {
    if (Data.size() && Data.back())
    {
      Data += '\0';
    }
  }

  inline bool IsAnsi() const
  {
    for (const unsigned char& ch : Data)
    {
      if (ch > 127)
      {
        return false;
      }
    }
    return true;
  }

  inline bool IsNumeric() const
  {
    for (size_t idx = 0; idx < Data.size(); ++idx)
    {
      if (Data[idx] < '0' || Data[idx] > '9')
      {
        return Data[idx] == '\0' && idx == Data.size() - 1;
      }
    }
    return true;
  }

  inline bool StartWith(const FString& s) const
  {
    if (Data.size() && s.Size())
    {
      if (Data.back() && !s.Back())
      {
        return Data._Starts_with(s.Data.substr(0, s.Data.size() - 1));
      }
      else if (!Data.back() && s.Back())
      {
        return Data.substr(0, Data.size() - 1)._Starts_with(s.Data);
      }
    }
    return Data._Starts_with(s.Data);
  }

  int Compare(size_t off, size_t count, const char* str) const
  {
    return Data.compare(off, count, str);
  }

  int Compare(size_t off, size_t count, const FString& str) const
  {
    if (Data.size() && str.Data.size())
    {
      if (Data.back() == 0 && str.Data.back() != 0)
      {
        if (off > 0)
        {
          off--;
        }
      }
      else if (Data.back() != 0 && str.Data.back() == 0)
      {
        count--;
      }
    }
    return Data.compare(off, count, str.Data.c_str());
  }

  size_t FindFirstOf(char ch, size_t off) const
  {
    return Data.find_first_of(ch, off);
  }

  size_t FindLastOf(char ch, size_t off = std::string::npos) const
  {
    return Data.find_last_of(ch, off);
  }

  static FString Sprintf(const char* fmt, ...)
  {
    int final_n, n = ((int)strlen(fmt)) * 2;
    char* formatted = nullptr;
    va_list ap;
    while (1)
    {
      if (formatted)
      {
        free(formatted);
      }
      formatted = (char*)malloc(n);
      strcpy(&formatted[0], fmt);
      va_start(ap, fmt);
      final_n = vsnprintf(&formatted[0], n, fmt, ap);
      va_end(ap);
      if (final_n < 0 || final_n >= n)
      {
        n += abs(final_n - n + 1);
      }
      else
      {
        break;
      }
    }
    FString r(formatted);
    if (formatted)
    {
      free(formatted);
    }
    return r;
  }

  inline std::string StringForHash() const
  {
    if (!Data.size())
    {
      return {};
    }
    if (Data.back() != 0)
    {
      std::string r = Data;
      r.resize(r.size() + 1);
      return r;
    }
    return Data;
  }

  // Returns valid UTF8 by triming '\0'
  inline std::string UTF8() const
  {
    if (!Data.size())
    {
      return {};
    }
    if (Data.back() != 0)
    {
      return Data;
    }
    return Data.substr(0, Data.size() - 1);
  }

  inline std::vector<FString> Split(char sep) const
  {
    std::vector<FString> result;
    int32 start = 0;
    int32 idx = 0;
    for (; idx < Data.size(); ++idx)
    {
      if (Data[idx] == sep)
      {
        if (idx > start)
        {
          result.emplace_back(Data.substr(start, idx - start));
        }
        start = idx + 1;
      }
    }
    if (idx > 0 && Data[idx-1] != sep && idx > start)
    {
      result.emplace_back(Data.substr(start, idx - start));
    }
    return result;
  }

private:
  std::string Data;
};

inline FString operator+(const char* a, const FString& b)
{
  return FString(a) += b;
}

inline FString operator+(const wchar* a, const FString& b)
{
  return FString(a) += b;
}

inline FString operator+(const std::string& a, const FString& b)
{
  return FString(a) += b;
}

inline FString operator+(const std::wstring& a, const FString& b)
{
  return FString(a) += b;
}

namespace std 
{
  template <>
  struct hash<FString>
  {
    std::size_t operator()(const FString& a) const
    {
      return std::hash<std::string>()(a.StringForHash());
    }
  };
}

class FStringRef {
public:
  FStringRef()
  {}

  ~FStringRef()
  {
    if (Cached)
    {
      delete Cached;
    }
  }

  FString GetString();

  FString GetString(FStream& s);

  FString* Cached = nullptr;
  FILE_OFFSET Size = -1;
  FILE_OFFSET Offset = -1;
  FPackage* Package = nullptr;
};