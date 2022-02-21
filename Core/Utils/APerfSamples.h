#pragma once
#include <chrono>
#include <Tera/Core.h> // for ENABLE_PERF_SAMPLE
#include "ALog.h"

// Performance measurement
#if _DEBUG || ENABLE_PERF_SAMPLE
#define PERF_START(ID) auto start##ID = std::chrono::high_resolution_clock::now()
#define PERF_END(ID) LogP("Perf %s: %dms", #ID, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start##ID).count())
#define PERF_START_I(ID) uint64 start##ID = 0
#define PERF_ITER_BEGIN(ID) auto _tmpStart##ID = std::chrono::high_resolution_clock::now()
#define PERF_ITER_END(ID) start##ID += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - _tmpStart##ID).count();
#define PERF_END_I(ID) LogP("Perf %s: %llums", #ID, start##ID)
#else
#define PERF_START(ID)
#define PERF_END(ID)
#define PERF_START_I(ID)
#define PERF_ITER_BEGIN(ID)
#define PERF_ITER_END(ID)
#define PERF_END_I(ID)
#endif