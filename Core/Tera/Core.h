#pragma once
#ifdef _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif
#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_CLIENTBLOCK
#endif

// --------------------------------------------------------------------
// Build configuration
// --------------------------------------------------------------------

// #define CUSTOM_BUILD ".1"
static const unsigned short APP_VER_MAJOR = 2;
static const unsigned short APP_VER_MINOR = 30;
static const unsigned int BUILD_NUMBER = (
#include "../../build_num.txt"
);

// Enables performance samples in Release builds
#define ENABLE_PERF_SAMPLE 1
// Obsolete
#define ALLOW_UI_PKG_SAVE 1
// If disabled names won't show FName numbers
// and may cause bugs related to objects search
#define USE_FNAME_NUMBERS 1
// Static DC allows almost instant serialization
// and avoids any unnecessary memory allocations
#define USE_STATIC_DC_4_EXPORT 1
// Cache composite mapper into *.re files
#define CACHE_COMPOSITE_MAP 0
// Cache contents of the S1Game folder
#define CACHE_S1GAME_CONTENTS 0
// Currently gives no benefits. May be used in the future. 
#define ADVANCED_TERRAIN_SERIALIZATION 0
// Export sound cues from ULevel
#define EXPERIMENTAL_SOUND_LEVEL_EXPORT 1
// Maximum number of files to open from a single file dialog
#define OPEN_OP_MAX_FILES 14

// Load minimum class packages to improve load time.
// Must not be used in Release builds!
#if _DEBUG
#define MINIMAL_CORE 1
#else
#define MINIMAL_CORE 0
#endif

// For testing only.
// GPU buffer has lower quality due to packed positions and half precision UVs,
// so its better to use CPU buffer instead. (Tera uses GPU buffer to render models)
#define USE_GPU_VERTEX_BUFFER 0

#if _DEBUG
// DUMP_PATH should be set in the ENV
#if defined(DUMP_PATH)
#define DUMP_OBJECTS 0
#define DUMP_PACKAGES 0
#define DUMP_MAPPERS 0
void DumpData(void* data, int size, const char* path);
#endif
#define MULTITHREADED_CLASS_SERIALIZATION 1
#define SERIALIZE_PROPERTIES 1
#define BUILD_SUFFIX "d"
#else
#define MULTITHREADED_CLASS_SERIALIZATION 1
#define SERIALIZE_PROPERTIES 1
#ifdef CUSTOM_BUILD
#define BUILD_SUFFIX CUSTOM_BUILD
#else
#define BUILD_SUFFIX ""
#endif
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
enum { MAX_TEXCOORDS = 4 };
enum { MAX_GPUSKIN_BONES = 75 };
enum { MSP_MAX = 1 };

#define PACKAGE_MAGIC 0x9E2A83C1
#define VER_TERA_CLASSIC 610
#define VER_TERA_MODERN 897
#define VER_TERA_FILEMOD 2

#ifndef M_PI
#define _USE_MATH_DEFINES
#include <cmath>
#endif

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
FString ClassFlagsToString(uint32 flags);
FString TextureCompressionSettingsToString(uint8 flags);

inline double Lerp(double a, double b, double alpha)
{
  return (a * (1.0f - alpha)) + (b * alpha);
}

inline double Bilerp(double a, double b, double c, double d, double x, double y)
{
  return Lerp(Lerp(a, b, x), Lerp(c, d, x), y);
}

float USRand();

float UFractional(float value);

int32 Trunc(float v);

std::string GetAppVersion();

void InitCRCTable();
uint32 CalculateStringCRC(const uint8* data, int32 size);
uint32 CalculateDataCRC(const void* data, int32 size, uint32 crc = 0);

// Check if the CPU has AVX2 instructions set. Mandatory for TGA and PNG export/import
bool HasAVX2();
// Check if RE has administrator privileges
bool IsElevatedProcess();
// Check if UAC is enabled and admin consent is on
bool IsUacEnabled();
// Check if RE needs elevation
bool NeedsElevation();

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

// Hangul to wide
std::wstring K2W(const char* str, int32 len = -1);
std::wstring K2W(const std::string& str);
// Wide to Hangul
std::string W2K(const wchar_t* str, int32 len = -1);
std::string W2K(const std::wstring& str);

// Get file's last modification date
uint64 GetFileTime(const std::wstring& path);

// Format like a C string
std::string Sprintf(const char* fmt, ...);
std::string Sprintf(const char* fmt, va_list ap);
std::string Sprintf(const wchar* fmt, va_list ap);
std::wstring Sprintf(const wchar* fmt, ...);
std::string Sprintf(const std::string fmt, ...);

void memswap(void* a, void* b, size_t size);

template<class T>
inline T Align(const T ptr, int32 alignment)
{
  return (T)(((int64)ptr + alignment - 1) & ~(alignment - 1));
}

FString GetTempDir();
FString GetTempFilePath();

FString GetClientVersionString(const FString& s1data);

#ifndef DBreak()
#if _DEBUG
#include <intrin.h>
#define DBreak() __debugbreak()
#define DBreakIf(test) if (test) __debugbreak()
#else
#define DBreak()
#define DBreakIf(expr)
#endif
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

// Performance measurement
#if ENABLE_PERF_SAMPLE
#define PERF_START(ID) auto start##ID = std::chrono::high_resolution_clock::now()
#define PERF_END(ID) LogE("Perf %s: %dms", #ID, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start##ID).count())
#define PERF_START_I(ID) uint64 start##ID = 0
#define PERF_ITER_BEGIN(ID) auto _tmpStart##ID = std::chrono::high_resolution_clock::now()
#define PERF_ITER_END(ID) start##ID += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - _tmpStart##ID).count();
#define PERF_END_I(ID) LogE("Perf %s: %llums", #ID, start##ID)
#else
#define PERF_START(ID)
#define PERF_END(ID)
#endif