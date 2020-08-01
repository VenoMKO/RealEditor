#pragma once

// Wrapper to keep track of '\0'
class FString {
public:
  FString()
  {}

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
    return *this == std::string(a);
  }

  inline bool operator==(const FString& a) const
  {
    if (Data.empty())
    {
      return a.Data.empty() ? true : a.Data.size() == 1 && a.Data.front() == 0;
    }
    else if (a.Data.empty())
    {
      return Data.empty() ? true : Data.size() == 1 && Data.front() == 0;
    }

    if (Data.back() == 0 && a.Data.back() != 0)
    {
      return Data.substr(0, Data.size() - 1) == a.Data;
    }
    else if (a.Data.back() == 0 && Data.back() != 0)
    {
      return a.Data.substr(0, a.Data.size() - 1) == Data;
    }
    return Data == a.Data;
  }

  inline bool operator==(const std::string& a) const
  {
    if (Data.empty())
    {
      return a.empty() ? true : a.size() == 1 && a.front() == 0;
    }
    else if (a.empty())
    {
      return Data.empty() ? true : Data.size() == 1 && Data.front() == 0;
    }
    
    if (Data.back() == 0 && a.back() != 0)
    {
      return Data.substr(0, Data.size() - 1) == a;
    }
    else if (a.back() == 0 && Data.back() != 0)
    {
      return a.substr(0, a.size() - 1) == Data;
    }
    return Data == a;
  }

  inline operator std::string() const
  {
    return Data;
  }

  inline operator std::wstring() const
  {
    return A2W(Data);
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
    return A2W(Data);
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

  inline bool IsAnsi() const
  {
    for (const char& ch : Data)
    {
      if (ch > 127)
      {
        return false;
      }
    }
    return true;
  }

private:
  std::string Data;
};