#include "Core.h"
#include <memory>
#include <string>
#include <stdexcept>
#include <iostream>

#define NOGDICAPMASKS
#define NOMENUS
#define NOATOM
#define NODRAWTEXT
#define NOKERNEL
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NOSHOWWINDOW
#define NOWINSTYLES
#define NOSYSMETRICS
#define NODEFERWINDOWPOS
#define NOMCX
#define NOCRYPT
#define NOTAPE
#define NOIMAGE
#define NOPROXYSTUB
#define NORPC
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <ppl.h>

#include "../MiniLZO/minilzo.h"

#define HEAP_ALLOC(var,size) lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]
static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);
#define COMPRESSED_BLOCK_SIZE 0x20000
#define COMPRESSED_BLOCK_MAGIC PACKAGE_MAGIC

std::string W2A(const wchar_t* str, int32 len)
{
  if (len == -1)
  {
    len = lstrlenW(str);
  }
  int32 size = WideCharToMultiByte(CP_UTF8, 0, str, len, NULL, 0, NULL, NULL);
  std::string result(size, 0);
  WideCharToMultiByte(CP_UTF8, 0, str, len, &result[0], size, NULL, NULL);
  return result;
}

std::string W2A(const std::wstring& str)
{
  return W2A(&str[0], (int32)str.length());
}

std::wstring A2W(const char* str, int32 len)
{
  if (len == -1)
  {
    len = (int32)strlen(str);
  }
  int32 size = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
  std::wstring result(size, 0);
  MultiByteToWideChar(CP_UTF8, 0, str, len, &result[0], size);
  return result;
}

std::wstring A2W(const std::string& str)
{
  return A2W(&str[0], (int32)str.length());
}

inline bool CaseInsCharCompare(char a, char b)
{
  return (towupper(a) == towupper(b));
}

inline bool CaseInsCharCompareW(wchar_t a, wchar_t b)
{
  return (towupper(a) == towupper(b));
}

bool Wstricmp(const std::string& a, const std::string& b)
{
  std::wstring wA = A2W(a);
  std::wstring wB = A2W(b);
  return (wA.size() == wB.size()) && (std::equal(wA.begin(), wA.end(), wB.begin(), CaseInsCharCompareW));
}

bool Stricmp(const std::string& a, const std::string& b)
{
  return (a.size() == b.size()) && (std::equal(a.begin(), a.end(), b.begin(), CaseInsCharCompare));
}

void UThrow(const std::string& msg)
{
  throw std::runtime_error(msg);
}

void DThrow(const std::string& msg)
{
#ifdef _DEBUG
  try
  {
    UThrow(msg);
  }
  catch (std::runtime_error& e)
  {
    DLog(e.what());
  }
#endif
}

bool IsAnsi(const std::string& str)
{
  std::wstring wstr = A2W(str);
  for (const wchar& ch : wstr)
  {
    if (ch > 127)
    {
      return false;
    }
  }
  return true;
}

std::string Sprintf(const std::string fmt, ...)
{
  int final_n, n = ((int)fmt.size()) * 2;
  std::unique_ptr<char[]> formatted;
  va_list ap;
  while (1)
  {
    formatted.reset(new char[n]);
    strcpy(&formatted[0], fmt.c_str());
    va_start(ap, fmt);
    final_n = vsnprintf(&formatted[0], n, fmt.c_str(), ap);
    va_end(ap);
    if (final_n < 0 || final_n >= n)
    {
      n += abs(final_n - n + 1);
    }
    else
    {
      break;
    }
  }
  return std::string(formatted.get());
}

void __Log(const std::string fmt, ...)
{
  std::string format = fmt + "\r\n";
  int final_n, n = ((int)fmt.size()) * 2;
  std::unique_ptr<char[]> formatted;
  va_list ap;
  while (1)
  {
    formatted.reset(new char[n]);
    strcpy(&formatted[0], fmt.c_str());
    va_start(ap, fmt);
    final_n = vsnprintf(&formatted[0], n, fmt.c_str(), ap);
    va_end(ap);
    if (final_n < 0 || final_n >= n)
    {
      n += abs(final_n - n + 1);
    }
    else
    {
      break;
    }
  }
  std::cout << formatted.get() << std::endl;
}

void LZO::Decompress(void* src, FILE_OFFSET srcSize, void* dst, FILE_OFFSET dstSize)
{
  lzo_bytep ptr = (lzo_bytep)src;
  lzo_bytep start = ptr;
  lzo_bytep dstStart = (lzo_bytep)dst;
  if (*(int*)ptr != COMPRESSED_BLOCK_MAGIC)
  {
    UThrow("Invalid or corrupted compression block!");
  }
  ptr += 4;
  uint32 blockSize = *(uint32*)ptr; ptr += 4;
  uint32 totalCompressedSize = *(uint32*)ptr; ptr += 4;
  uint32 totalDecompressedSize = *(uint32*)ptr; ptr += 4;
  int32 totalBlocks = (int32)ceilf((float)totalDecompressedSize / (float)blockSize);

  static bool err = (LZO_E_OK != lzo_init());

  if (err)
  {
    UThrow("Failed to initialize LZO!");
  }

  struct CompressionBlock {
    uint32 srcOffset = 0;
    uint32 srcSize = 0;
    uint32 dstOffset = 0;
    uint32 dstSize = 0;
  };

  const uint32 chunkHeaderSize = 16;
  const uint32 blockHeaderSize = 8;
  uint32 compressedOffset = 0;
  uint32 decompressedOffset = 0;

  CompressionBlock* compressionInfo = new CompressionBlock[totalBlocks];
  for (int i = 0; i < totalBlocks; i++)
  {
    compressionInfo[i].srcOffset = chunkHeaderSize + compressedOffset + totalBlocks * blockHeaderSize;
    compressionInfo[i].srcSize = *(uint32*)ptr; ptr += 4;
    compressionInfo[i].dstSize = *(uint32*)ptr; ptr += 4;
    compressionInfo[i].dstOffset = decompressedOffset;
    compressedOffset += compressionInfo[i].srcSize;
    decompressedOffset += compressionInfo[i].dstSize;
  }

  concurrency::parallel_for(int32(0), totalBlocks, [&](int32 i) {
    lzo_uint decompressedSize = compressionInfo[i].dstSize;
    int e = lzo1x_decompress_safe(start + compressionInfo[i].srcOffset, compressionInfo[i].srcSize, dstStart + compressionInfo[i].dstOffset, &decompressedSize, NULL);
    if (e != LZO_E_OK)
    {
      UThrow("Corrupted compression block. Code: " + std::to_string(e));
    }
  });

  delete[] compressionInfo;
}
