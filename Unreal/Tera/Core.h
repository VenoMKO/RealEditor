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
#include "UNames.h"

typedef int32 FILE_OFFSET;
typedef int32 NAME_INDEX;
typedef int32 PACKAGE_INDEX;
typedef int32 NET_INDEX;
typedef unsigned long BITFIELD;

enum { INDEX_NONE = -1 };

#define PACKAGE_MAGIC 0x9E2A83C1
#define VER_TERA_CLASSIC 610
#define VER_TERA_MODERN 897

#include <string>
// Use std::vector instead of TArray
#include <vector>
// Use std::map instead of TMap
#include <map>

// --------------------------------------------------------------------
// Forward
// --------------------------------------------------------------------

class ALog;

class	UObject;
class		UComponent;
class		UField;
class			UConst;
class			UEnum;
class			UProperty;
class				UByteProperty;
class				UIntProperty;
class				UBoolProperty;
class				UFloatProperty;
class				UObjectProperty;
class					UComponentProperty;
class					UClassProperty;
class					UInterfaceProperty;
class				UNameProperty;
class				UStructProperty;
class               UStrProperty;
class               UArrayProperty;
class				UMapProperty;
class				UDelegateProperty;
class			UStruct;
class				UFunction;
class				UState;
class					UClass;
class				UScriptStruct;
class		UTextBuffer;
class		UObjectRedirector;


class FName;
class FStream;
class FPackage;
class FString;
class FStateFrame;
class FObjectResource;
class FObjectImport;
class FObjectExport;

// --------------------------------------------------------------------
// Utils
// --------------------------------------------------------------------

#define SET_PACKAGE(s, obj) if (s.IsReading() && s.GetPackage()) obj.Package = s.GetPackage()

FString ObjectFlagsToString(uint64 flags);
FString ExportFlagsToString(uint32 flags);
FString PixelFormatToString(uint32 pf);
FString PackageFlagsToString(uint32 flags);

// Generic runtime error
void UThrow(const char* fmt, ...);
// Check if a string needs to be converted to wstring
bool IsAnsi(const std::string& str);
bool IsAnsi(const std::wstring& str);
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
// Get file's last modification date
uint64 GetFileTime(const std::wstring& path);

// Format like a C string
std::string Sprintf(const char* fmt, ...);
std::string Sprintf(const std::string fmt, ...);

void memswap(void* a, void* b, size_t size);

#define Check(expr) if (!expr) UThrow(std::string(std::string(strrchr("\\" __FILE__, '\\') + 1) + ":" + std::to_string(__LINE__)).c_str())

#ifdef _DEBUG
inline void DBreak()
{
  // BP here
  int test = 1;
}

inline void DBreakIf(bool test) { if (test) DBreak(); }
#else
#define DBreak()
#define DBreakIf(expr)
#endif

// --------------------------------------------------------------------
// Compression
// --------------------------------------------------------------------

namespace LZO
{
  void Decompress(void* src, FILE_OFFSET srcSize, void* dst, FILE_OFFSET dstSize, bool concurrent = true);
}

// --------------------------------------------------------------------
// Logging
// --------------------------------------------------------------------

#define LogI(...) ALog::ILog(Sprintf(__VA_ARGS__))
#define LogW(...) ALog::WLog(Sprintf(__VA_ARGS__))
#define LogE(...) ALog::ELog(Sprintf(__VA_ARGS__))

// --------------------------------------------------------------------
// Misc
// --------------------------------------------------------------------

#ifdef _DEBUG
#define PERF_START(ID) auto start##ID = std::chrono::high_resolution_clock::now()
#define PERF_END(ID) LogE("Perf %s: %d", #ID, std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start##ID).count())
#else
#define PERF_START(ID)
#define PERF_END(ID)
#endif

#if defined(DUMP_PATH) && _DEBUG
#define DUMP_OBJECTS 0
#define DUMP_PACKAGES 0
#define DUMP_MAPPERS 0
#endif