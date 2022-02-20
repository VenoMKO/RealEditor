#pragma once
#include <string>
// Check if a string needs to be converted to wstring
bool IsAnsi(const std::string& str);
bool IsAnsi(const std::wstring& str);

// Wide string to UTF8
std::string W2A(const wchar_t* str, int32 len = -1);
std::string W2A(const std::wstring& str);

// UTF8 string to wide
std::wstring A2W(const char* str, int32 len = -1);
std::wstring A2W(const std::string& str);

// Hangul to wide
std::wstring K2W(const char* str, int32 len = -1);
std::wstring K2W(const std::string& str);

// Wide to Hangul
std::string W2K(const wchar_t* str, int32 len = -1);
std::string W2K(const std::wstring& str);

// Format like a C string
std::string Sprintf(const char* fmt, ...);
std::string Sprintf(const char* fmt, va_list ap);
std::string Sprintf(const wchar* fmt, va_list ap);
std::wstring Sprintf(const wchar* fmt, ...);
std::string Sprintf(const std::string fmt, ...);