#include "FStream.h"
#include "FPackage.h"

#include <ppl.h>

FStream& FStream::operator<<(FString& s)
{
  if (IsReading())
  {
    s.Resize(0);
    int32 len = 0;
    (*this) << len;
    if (len > 0)
    {
      s.Resize(len);
      SerializeBytes(&s[0], len);
    }
    else if (len < 0)
    {
      len = -len * 2;
      wchar* data = (wchar*)malloc(len);
      SerializeBytes(data, len);
      s += data;
      free(data);
    }
  }
  else
  {
    int32 len = (int32)s.Size();
    if (!s.IsAnsi())
    {
      std::wstring wstr = s;
      len = -(int32)wstr.size();
      (*this) << len;
      SerializeBytes((void*)s.C_str(), len * -2);
    }
    else
    {
      (*this) << len;
      SerializeBytes((void*)s.C_str(), len);
    }
  }
  return *this;
}

uint16 FStream::GetFV() const
{
  return Package ? Package->GetFileVersion() : 0;
}

uint16 FStream::GetLV() const
{
  return Package ? Package->GetLicenseeVersion() : 0;
}

FStream& FStream::operator<<(FStringRef& r)
{
  if (IsReading())
  {
    r.Package = GetPackage();
    r.Offset = GetPosition();
    (*this) << r.Size;
    SetPosition(r.Offset + sizeof(FILE_OFFSET) + r.Size);
  }
  else
  {
    if (r.Cached->Empty())
    {
      r.GetString();
    }
    (*this) << *r.Cached;
  }
  return *this;
}

void FStream::SerializeCompressed(void* v, int32 length, ECompressionFlags flags, bool concurrent)
{
  if (IsReading())
  {
    FCompressedChunkInfo packageFileTag;
    *this << packageFileTag;
    FCompressedChunkInfo summary;
    *this << summary;

    DBreakIf(packageFileTag.CompressedSize != PACKAGE_MAGIC);

    int32 loadingCompressionChunkSize = packageFileTag.DecompressedSize;
    if (loadingCompressionChunkSize == PACKAGE_MAGIC)
    {
      loadingCompressionChunkSize = COMPRESSED_BLOCK_SIZE;
    }

    int32	totalChunkCount = (summary.DecompressedSize + loadingCompressionChunkSize - 1) / loadingCompressionChunkSize;
    FCompressedChunkInfo* chunkInfo = new FCompressedChunkInfo[totalChunkCount];

    if (concurrent && totalChunkCount > 1)
    {
      void** compressedDataChunks = new void*[totalChunkCount];
      for (int32 idx = 0; idx < totalChunkCount; ++idx)
      {
        *this << chunkInfo[idx];
        if (idx)
        {
          chunkInfo[idx].DecompressedOffset = chunkInfo[idx - 1].DecompressedOffset + chunkInfo[idx - 1].DecompressedSize;
        }
      }
      for (int32 idx = 0; idx < totalChunkCount; ++idx)
      {
        compressedDataChunks[idx] = malloc(chunkInfo[idx].CompressedSize);
        SerializeBytes(compressedDataChunks[idx], chunkInfo[idx].CompressedSize);
      }

      uint8* dest = (uint8*)v;
      std::atomic_bool err = {false};
      concurrency::parallel_for(int32(0), totalChunkCount, [&](int32 idx) {
        const FCompressedChunkInfo& chunk = chunkInfo[idx];
        if (err.load())
        {
          return;
        }
        if (!DecompressMemory(flags, dest + chunk.DecompressedOffset, chunk.DecompressedSize, compressedDataChunks[idx], chunk.CompressedSize))
        {
          err.store(true);
        }
      });

      for (int32 idx = 0; idx < totalChunkCount; ++idx)
      {
        free(compressedDataChunks[idx]);
      }

      delete[] chunkInfo;
      delete[] compressedDataChunks;

      if (err.load())
      {
        UThrow("Failed to decompress data!");
      }
    }
    else
    {
      int32 maxCompressedSize = 0;
      for (int32 idx = 0; idx < totalChunkCount; ++idx)
      {
        *this << chunkInfo[idx];
        maxCompressedSize = std::max(chunkInfo[idx].CompressedSize, maxCompressedSize);
      }

      uint8* dest = (uint8*)v;
      void* compressedBuffer = malloc(maxCompressedSize);
      bool err = false;
      for (int32 idx = 0; idx < totalChunkCount; ++idx)
      {
        const FCompressedChunkInfo& chunk = chunkInfo[idx];
        SerializeBytes(compressedBuffer, chunk.CompressedSize);
        if (!DecompressMemory(flags, dest, chunk.DecompressedSize, compressedBuffer, chunk.CompressedSize))
        {
          err = true;
          break;
        }
        dest += chunk.DecompressedSize;
      }

      free(compressedBuffer);
      delete[] chunkInfo;

      if (err)
      {
        UThrow("Failed to decompress data!");
      }
    }
  }
}
