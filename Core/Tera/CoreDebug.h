#pragma once
// Memory
#ifdef _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif
#ifndef DEBUG_CLIENTBLOCK
#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_CLIENTBLOCK
#endif
#endif

// Allows to dump GPKs
// DUMP_PATH - directory to save dumps to. Should be set in the ENV
#if _DEBUG && defined(DUMP_PATH)
#define DUMP_OBJECTS 0
#define DUMP_PACKAGES 0
#define DUMP_MAPPERS 0
void DumpData(void* data, int size, const char* path);
#endif

// Programmable breakpoints
#if _DEBUG
#include <intrin.h>
#define DBreak() __debugbreak()
#define DBreakIf(test) if (test) __debugbreak()
#else
#define DBreak()
#define DBreakIf(expr)
#endif