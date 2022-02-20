#pragma once
// --------------------------------------------------------------------
// Build configuration
// --------------------------------------------------------------------

// Enables performance samples in Release builds
#define ENABLE_PERF_SAMPLE 1

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

// Use LzoPro instead of MiniLzo
#define USE_LZOPRO 1

// LzoPro compression level 0 - 10(low compression(fast) - high compression(slow))
#define LZOPRO_COMPRESSION_LEVEL 10

// Allow concurrent compression/decompression
#define ALLOW_CONCURRENT_LZO 1

// Compress texture MipMaps
#define TEXTURE2D_COMPRESSION 1

// Add NULL-terminator to new FNameEntries
#define TERMINATE_NEW_NAMES 1

// Add NULL-terminator to existing FNameEntries on edit
#define TERMINATE_EXISTING_NAMES 1

// Allow internal flags edits
#define INTERNAL_FLAGS _DEBUG

// Load minimum class packages to improve load time.
// Must not be used in Release builds!
#define MINIMAL_CORE _DEBUG

// For testing only.
// GPU buffer has lower quality due to packed positions and half precision UVs,
// so its better to use CPU buffer instead. (Tera uses GPU buffer to render models)
#define USE_GPU_VERTEX_BUFFER 0

// Build and save SkelMesh's raw points index buffers
#define SAVE_RAWINDICES 0

// Allows mod compression
#define USE_MOD_COMPRESSION 1

// Parallel MOD compression
#define CONCURRENT_MOD_COMPRESSION 1

// Enable concurrent *.u serialization
#define MULTITHREADED_CLASS_SERIALIZATION 1

// For testing and debugging. Allows to turn off properties.
#define SERIALIZE_PROPERTIES 1

// Allows to disable modern TMM file format
#define USE_LEGACY_FILEMOD_VER 1

#include "CoreDebug.h"
#include "CoreTypes.h"
#include "CoreVersion.h"
#include "CoreStrings.h"

FString ObjectFlagsToString(uint64 flags);
FString ExportFlagsToString(uint32 flags);
FString PixelFormatToString(uint32 pf);
FString PackageFlagsToString(uint32 flags);
FString ClassFlagsToString(uint32 flags);
FString TextureCompressionSettingsToString(uint8 flags);

// Check if the CPU has AVX2 instructions set. Mandatory for TGA and PNG export/import
bool HasAVX2();

// Check if RE needs elevation
bool NeedsElevation();

// Generic runtime errors
void UThrow(const char* fmt, ...);
void UThrow(const wchar* fmt, ...);

template<class T>
inline T Align(const T ptr, int32 alignment)
{
  return (T)(((int64)ptr + alignment - 1) & ~(alignment - 1));
}

FString GetTempDir();
FString GetTempFilePath();
// Get file's last modification date
uint64 GetFileTime(const std::wstring& path);

#define LogI(...) ALog::ILog(Sprintf(__VA_ARGS__))
#define LogW(...) ALog::WLog(Sprintf(__VA_ARGS__))
#define LogE(...) ALog::ELog(Sprintf(__VA_ARGS__))

#include <chrono>
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
#define PERF_START_I(ID)
#define PERF_ITER_BEGIN(ID)
#define PERF_ITER_END(ID)
#define PERF_END_I(ID)
#endif

#define MAKE_IDB(name) "#" wxSTRINGIZE(## name)

#ifndef M_PI
#define _USE_MATH_DEFINES
#include <cmath>
#endif
