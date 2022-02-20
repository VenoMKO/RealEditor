#pragma once
#include <intrin.h>

inline double Lerp(double a, double b, double alpha)
{
  return (a * (1.0f - alpha)) + (b * alpha);
}

inline double Bilerp(double a, double b, double c, double d, double x, double y)
{
  return Lerp(Lerp(a, b, x), Lerp(c, d, x), y);
}

float USRand();

inline int32 Trunc(float v)
{
  return _mm_cvtt_ss2si(_mm_set_ss(v));
}

inline float UFractional(float value)
{
  return value - Trunc(value);
}

void InitCRCTable();
uint32 CalculateStringCRC(const uint8* data, int32 size);
uint32 CalculateDataCRC(const void* data, int32 size, uint32 crc = 0);