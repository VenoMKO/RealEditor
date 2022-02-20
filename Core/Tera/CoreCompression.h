#pragma once
#define COMPRESSED_BLOCK_SIZE 0x20000
#define COMPRESSED_CHUNK_SIZE 0x100000

// Legacy macOS's RE decompression. src must contain chunks.
namespace LZO
{
  void Decompress(const void* src, FILE_OFFSET srcSize, void* dst, FILE_OFFSET dstSize, bool concurrent = true);
  void Compress(const void* src, FILE_OFFSET srcSize, void* dst, FILE_OFFSET& dstSize, bool concurrent = true);
}

// CompressedBuffer must point directly to a compressed data block
bool DecompressMemory(ECompressionFlags flags, void* decompressedBuffer, int32 decompressedSize, const void* compressedBuffer, int32 compressedSize);
bool CompressMemory(ECompressionFlags flags, void* compressedBuffer, int32* compressedSize, const void* decompressedBuffer, int32 decompressedSize);
