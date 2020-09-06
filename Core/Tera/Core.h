#pragma once
#ifdef _DEBUG
#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_CLIENTBLOCK
#endif

// --------------------------------------------------------------------
// Build configuration
// --------------------------------------------------------------------

static const float APP_VER = 0.21f;

#define WIN_POS_FULLSCREEN INT_MIN
#define WIN_POS_CENTER INT_MIN + 1

#define ENABLE_PERF_SAMPLE 1
#define ALLOW_UI_PKG_SAVE 1

#define CACHE_COMPOSITE_MAP 0
#define CACHE_S1GAME_CONTENTS 0

#if _DEBUG
// DUMP_PATH should be set in the ENV
#if defined(DUMP_PATH)
#define DUMP_OBJECTS 0
#define DUMP_PACKAGES 0
#define DUMP_MAPPERS 1
void DumpData(void* data, int size, const char* path);
#endif
#define MULTITHREADED_CLASS_SERIALIZATION 0
#define SERIALIZE_PROPERTIES 1
#else
#define MULTITHREADED_CLASS_SERIALIZATION 1
#define SERIALIZE_PROPERTIES 1
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

typedef uint32 ubool;

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

#include <chrono>

// --------------------------------------------------------------------
// Forward
// --------------------------------------------------------------------

class FName;
class FStream;
class FPackage;
class FString;
class FStateFrame;
class FObjectResource;
class FObjectImport;
class FObjectExport;
class VObjectExport;

class	UObject;
class UComponent;
class UField;
class UConst;
class UEnum;
class UProperty;
class UByteProperty;
class UIntProperty;
class UBoolProperty;
class UFloatProperty;
class UObjectProperty;
class UComponentProperty;
class UClassProperty;
class UInterfaceProperty;
class UNameProperty;
class UStructProperty;
class UStrProperty;
class UArrayProperty;
class UMapProperty;
class UDelegateProperty;
class UStruct;
class UFunction;
class UState;
class UClass;
class UScriptStruct;
class UTextBuffer;
class UObjectRedirector;

// --------------------------------------------------------------------
// Utils
// --------------------------------------------------------------------

// Glue for URefs. Don't use.
#define __GLUE_OBJ_REF(Name, Sfx)\
  Name##Sfx

// Serialize UObject* with its index
// In case we fail to find the object, we won't lose its index when saving
#define SERIALIZE_UREF(Stream, Field)\
  Stream.SerializeObjectRef((void*&)Field, __GLUE_OBJ_REF(Field, RefIndex))

// Declare an object with its index
#define DECL_UREF(TClass, Name)\
  TClass* Name = nullptr;\
  PACKAGE_INDEX __GLUE_OBJ_REF(Name, RefIndex) = 0

FString ObjectFlagsToString(uint64 flags);
FString ExportFlagsToString(uint32 flags);
FString PixelFormatToString(uint32 pf);
FString PackageFlagsToString(uint32 flags);

// Generic runtime error
void UThrow(const char* fmt, ...);
void UThrow(const wchar* fmt, ...);
// Check if a string needs to be converted to wstring
bool IsAnsi(const std::string& str);
bool IsAnsi(const std::wstring& str);
// Wide string to UTF8
std::string W2A(const wchar_t* str, int32 len = -1);
std::string W2A(const std::wstring& str);
// UTF8 string to wide
std::wstring A2W(const char* str, int32 len = -1);
std::wstring A2W(const std::string& str);
// Get file's last modification date
uint64 GetFileTime(const std::wstring& path);

// Format like a C string
std::string Sprintf(const char* fmt, ...);
std::string Sprintf(const char* fmt, va_list ap);
std::string Sprintf(const wchar* fmt, va_list ap);
std::wstring Sprintf(const wchar* fmt, ...);
std::string Sprintf(const std::string fmt, ...);

void memswap(void* a, void* b, size_t size);

#if _DEBUG
inline void DBreak()
{
  // BP here
  int stop = 1;
}

inline void DBreakIf(bool test) { if (test) DBreak(); }
#else
#define DBreak()
#define DBreakIf(expr)
#endif

// --------------------------------------------------------------------
// Compression
// --------------------------------------------------------------------

#define COMPRESSED_BLOCK_SIZE 0x20000

// Legacy macOS's RE decompression. src must contain chunks.
namespace LZO
{
  void Decompress(const void* src, FILE_OFFSET srcSize, void* dst, FILE_OFFSET dstSize, bool concurrent = true);
}

// New decompression. compressedBuffer must point directly to a compressed data block
bool DecompressMemory(ECompressionFlags flags, void* decompressedBuffer, int32 decompressedSize, const void* compressedBuffer, int32 compressedSize);

bool CompressMemory(ECompressionFlags flags, void* compressedBuffer, int32* compressedSize, const void* decompressedBuffer, int32 decompressedSize);

// --------------------------------------------------------------------
// Logging
// --------------------------------------------------------------------

#define LogI(...) ALog::ILog(Sprintf(__VA_ARGS__))
#define LogW(...) ALog::WLog(Sprintf(__VA_ARGS__))
#define LogE(...) ALog::ELog(Sprintf(__VA_ARGS__))

// --------------------------------------------------------------------
// Misc
// --------------------------------------------------------------------

// Performance measure
#if ENABLE_PERF_SAMPLE
#define PERF_START(ID) auto start##ID = std::chrono::high_resolution_clock::now()
#define PERF_END(ID) LogE("Perf %s: %dms", #ID, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start##ID).count())
#else
#define PERF_START(ID)
#define PERF_END(ID)
#endif