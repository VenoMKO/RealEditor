#pragma once
#ifndef VER_TERA_FILEMOD
#define USE_LEGACY_FILEMOD_VER 1

// Current TMM version
#if USE_LEGACY_FILEMOD_VER
#define VER_TERA_FILEMOD 2
#else
#define VER_TERA_FILEMOD 3
#endif

// Added ability to embed TFCs into a mod GPK
#define VER_TERA_FILEMOD_ADD_TFC 2
// Added description to a mod GPK
#define VER_TERA_FILEMOD_ADD_DESCRIPTORS 3
#endif