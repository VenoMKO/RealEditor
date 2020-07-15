#pragma once

#ifdef _DEBUG
#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_CLIENTBLOCK
#endif

// --------------------------------------------------------------------
// Types
// --------------------------------------------------------------------

typedef	signed char int8;
typedef unsigned char uint8;

typedef signed short int int16;
typedef unsigned short int uint16;

typedef signed int int32;
typedef unsigned int uint32;

typedef signed long long int64;
typedef unsigned long long uint64;

typedef wchar_t wchar;

#include "FFlags.h"
typedef enum EPackageFlags EPackageFlags;
typedef enum EObjectFlags EObjectFlags;
typedef enum EFExportFlags EFExportFlags;

typedef int32 FILE_OFFSET;
typedef int32 NAME_INDEX;
typedef int32 PACKAGE_INDEX;
typedef int32 NET_INDEX;

enum { INDEX_NONE = -1 };
enum { NET_INDEX_NONE = 0 };

#define PACKAGE_MAGIC 0x9E2A83C1

// Use UTF-8 encoded std::string as internal string storage
#include <string>
// Use std::vector instead of TArray
#include <vector>
// Use std::map instead of TMap
#include <map>

// --------------------------------------------------------------------
// Forward
// --------------------------------------------------------------------

class FStream;
class FPackage;
class FName;
class FStateFrame;
class FObjectResource;
class FObjectImport;
class FObjectExport;


class UObject;
class UField;
class UStruct;
class UState;
class UClass;
class UEnum;
class UProperty;

// --------------------------------------------------------------------
// Utils
// --------------------------------------------------------------------

#define SET_PACKAGE(s, obj) if (s.IsReading() && s.GetPackage()) obj.Package = s.GetPackage()

// Generic runtime error
void UThrow(const std::string& msg);
// Debug error. Recoverable
void DThrow(const std::string& msg);
// Check if a string needs to be converted to wstring
bool IsAnsi(const std::string& str);
// Wide string to UTF8
std::string W2A(const wchar_t* str, int32 len = -1);
std::string W2A(const std::wstring& str);
// UTF8 string to wide
std::wstring A2W(const char* str, int32 len = -1);
std::wstring A2W(const std::string& str);
// Case-insensitive UTF8 string comparison
bool Wstricmp(const std::string& a, const std::string& b);
// Case-insensitive string comparison
bool Stricmp(const std::string& a, const std::string& b);

// Format like a C string
std::string Sprintf(const std::string fmt, ...);

// --------------------------------------------------------------------
// Compression
// --------------------------------------------------------------------

namespace LZO
{
  void Decompress(void* src, FILE_OFFSET srcSize, void* dst, FILE_OFFSET dstSize);
}

#define LogI(...) ALog::ILog(Sprintf(__VA_ARGS__))
#define LogW(...) ALog::WLog(Sprintf(__VA_ARGS__))
#define LogE(...) ALog::ELog(Sprintf(__VA_ARGS__))