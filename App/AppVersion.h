#pragma once
static const unsigned short APP_VER_MAJOR = 2;
static const unsigned short APP_VER_MINOR = 40;
// #define CUSTOM_BUILD ".1"
static const unsigned int BUILD_NUMBER = (
  #include "../../build_num.txt"
);

// Version suffix
#if _DEBUG
#define BUILD_SUFFIX "d"
#else
#ifdef CUSTOM_BUILD
#define BUILD_SUFFIX CUSTOM_BUILD
#else
#define BUILD_SUFFIX ""
#endif
#endif

std::string GetAppVersion();