#include "TextureProcessor.h"

#include <Utils/ALog.h>
#include <Tera/FStream.h>

#include <nvtt/nvtt.h>
#include <FreeImage.h>

#include <ppl.h>
#include <algorithm>
#include <filesystem>

#include "DDS.h"

// freeimage raii container
struct FreeImageHolder {
  FreeImageHolder(bool initContext)
    : ctx(initContext)
  {
    if (ctx)
    {
      FreeImage_Initialise();
    }
  }

  FIBITMAP* bmp = nullptr;
  FIMEMORY* mem = nullptr;
  bool ctx = false;

  ~FreeImageHolder()
  {
    FreeImage_Unload(bmp);
    FreeImage_CloseMemory(mem);
    if (ctx)
    {
      FreeImage_DeInitialise();
    }
  }
};

// nvtt handler
struct TPOutputHandler : public nvtt::OutputHandler {
  static const int32 MaxMipCount = 16;

  struct Mip {
    int32 SizeX = 0;
    int32 SizeY = 0;

    int32 Size = 0;
    int32 LastOffset = 0;
    void* Data = nullptr;
  };

  ~TPOutputHandler() override
  {
    for (int32 idx = 0; idx < MaxMipCount; ++idx)
    {
      free(Mips[idx].Data);
    }
  }

  void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
  {
    if (miplevel >= MaxMipCount || !Ok)
    {
      Ok = false;
      return;
    }
    Mip& mip = Mips[MipsCount];
    mip.Size = size;
    mip.SizeX = width;
    mip.SizeY = height;
    if (size)
    {
      mip.Data = malloc(size);
    }
    DBreakIf(depth != 1);
    MipsCount++;
  }

  bool writeData(const void* data, int size) override
  {
    if (!Ok || !MipsCount || !data)
    {
      return Ok;
    }
    Mip& mip = Mips[MipsCount - 1];
    if (mip.LastOffset + size > mip.Size)
    {
      if (mip.Size)
      {
        if (void* tmp = realloc(mip.Data, mip.LastOffset + size))
        {
          mip.Size = mip.LastOffset + size;
          mip.Data = tmp;
        }
        else
        {
          Ok = false;
        }
      }
      else
      {
        mip.Data = malloc(size);
        mip.Size = size;
      }
    }
    if (Ok && mip.Data)
    {
      void* dst = (uint8*)mip.Data + mip.LastOffset;
      memcpy(dst, data, size);
      mip.LastOffset += size;
    }
    return Ok;
  }

  void endImage() override
  {}

  Mip Mips[MaxMipCount];
  int32 MipsCount = 0;
  bool Ok = true;
};

nvtt::MipmapFilter MipFilterTypeToNvtt(MipFilterType type)
{
  switch (type)
  {
  case MipFilterType::Box:
    return nvtt::MipmapFilter_Box;
  case MipFilterType::Triangle:
    return nvtt::MipmapFilter_Triangle;
  case MipFilterType::Kaiser:
  default:
    return nvtt::MipmapFilter_Kaiser;
  }
}

nvtt::WrapMode AddressModeToNvtt(TextureAddress mode)
{
  switch (mode)
  {
  case TA_Clamp:
    return nvtt::WrapMode_Clamp;
  case TA_Mirror:
    return nvtt::WrapMode_Mirror;
  case TA_Wrap:
  default:
    return nvtt::WrapMode_Repeat;
    break;
  }
}

bool TextureProcessor::Process()
{
  if (InputPath.empty())
  {
    if ((!InputData || !InputDataSize) && (!InIsCube || !InputCubeIsValid()))
    {
      Error = "Texture Processor: no input data";
      return false;
    }
    
    if (OutputPath.empty())
    {
      if (!OutputData || !OutputDataSize)
      {
        Error = "Texture Processor: no output specified";
        return false;
      }
      return BytesToBytes();
    }
    return BytesToFile();
  }
  if (OutputPath.empty())
  {
    return FileToBytes();
  }
  Error = "Texture Processor: no output specified";
  return false;
}

bool TextureProcessor::UnfoldCube()
{
  nvtt::Format fmt = nvtt::Format_Count;
  switch (InputFormat)
  {
  case TCFormat::DXT1:
    fmt = nvtt::Format_DXT1;
    break;
  case TCFormat::DXT3:
    fmt = nvtt::Format_DXT3;
    break;
  case TCFormat::DXT5:
    fmt = nvtt::Format_DXT5;
    break;
  default:
  case TCFormat::ARGB8:
    break;
  }

  nvtt::Surface surface;
  nvtt::CubeSurface cube;
  LogI("Texture Processor: Setting cube surface image");
  for (int32 face = 0; face < InputCube.size(); ++face)
  {
    nvtt::Surface& s = cube.face(face);
    bool result = false;
    if (fmt == nvtt::Format_Count)
    {
      result = s.setImage(nvtt::InputFormat_BGRA_8UB, InputCube[face].X, InputCube[face].Y, 1, InputCube[face].Data);
    }
    else
    {
      result = s.setImage2D(fmt, nvtt::Decoder_D3D9, InputCube[face].X, InputCube[face].Y, InputCube[face].Data);
    }
    if (!result || s.isNull() || !s.width() || !s.height())
    {
      Error = "Texture Processor: failed to create input cube surface";
      return false;
    }
    s.flipY();
  }
  surface = cube.unfold(nvtt::CubeLayout_Row);
  if (surface.isNull() || !surface.width() || !surface.height())
  {
    Error = "Texture Processor: failed to unfold the input cube surface";
    return false;
  }

  nvtt::Format ofmt = nvtt::Format_RGBA;
  switch (OutputFormat)
  {
  case TCFormat::DXT1:
    ofmt = nvtt::Format_DXT1;
    break;
  case TCFormat::DXT3:
    ofmt = nvtt::Format_DXT3;
    break;
  case TCFormat::DXT5:
    ofmt = nvtt::Format_DXT5;
    break;
  default:
  case TCFormat::ARGB8:
    break;
  }

  nvtt::Context ctx;
  nvtt::CompressionOptions compressionOptions;
  compressionOptions.setFormat(ofmt);
  if (ofmt == nvtt::Format_RGBA)
  {
    compressionOptions.setPixelFormat(32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);
  }

  nvtt::OutputOptions outputOptions;
  TPOutputHandler ohandler;
  outputOptions.setSrgbFlag(SRGB);
  outputOptions.setOutputHandler(&ohandler);

  if (!ctx.compress(surface, 0, 1, compressionOptions, outputOptions))
  {
    Error = "Texture Processor: Failed to compress cube surface!";
    return false;
  }

  for (int32 idx = 0; idx < ohandler.MaxMipCount; ++idx)
  {
    OutputDataSize += ohandler.Mips[idx].Size;
  }

  if(!(OutputMipCount = ohandler.MipsCount))
  {
    Error = "Texture Processor: failed to compress the bitmap. No mipmaps were generated.";
    return false;
  }

  if (!OutputDataSize)
  {
    Error = "Texture Processor: NVTT encoding error!";
    return false;
  }

  OutputData = malloc(OutputDataSize);

  int32 offset = 0;
  for (int32 idx = 0; idx < OutputMipCount; ++idx)
  {
    TPOutputHandler::Mip& mip = ohandler.Mips[idx];
    memcpy((uint8*)OutputData + offset, mip.Data, mip.Size);
    OutputMips.push_back({ mip.SizeX, mip.SizeY, mip.Size, (uint8*)OutputData + offset });
    offset += ohandler.Mips[idx].Size;
  }

  return true;
}

bool TextureProcessor::BytesToFile()
{
  nvtt::Surface surface;
  bool hasAlpha = true;
  if (OutputFormat == TCFormat::DDS)
  {
    return BytesToDDS();
  }
  if (!HasAVX2())
  {
    Error = "Texture Processor: Your CPU does not support AVX2 instructions. Please, use DDS format to export the texture.";
    return false;
  }
  if (InputFormat == TCFormat::DXT1 || InputFormat == TCFormat::DXT3 || InputFormat == TCFormat::DXT5 || InputFormat == TCFormat::ARGB8)
  {
    nvtt::Format fmt = nvtt::Format_Count;
    switch (InputFormat)
    {
    case TCFormat::DXT1:
      fmt = nvtt::Format_DXT1;
      hasAlpha = false;
      break;
    case TCFormat::DXT3:
      fmt = nvtt::Format_DXT3;
      break;
    case TCFormat::DXT5:
      fmt = nvtt::Format_DXT5;
      break;
    default:
    case TCFormat::ARGB8:
      hasAlpha = true;
      break;
    }

    if (InIsCube)
    {
      nvtt::CubeSurface cube;
      hasAlpha = false;
      LogI("Texture Processor: Setting cube surface image");
      for (int32 face = 0; face < InputCube.size(); ++face)
      {
        nvtt::Surface& s = cube.face(face);
        bool result = false;
        if (fmt == nvtt::Format_Count)
        {
          result = s.setImage(nvtt::InputFormat_BGRA_8UB, InputCube[face].X, InputCube[face].Y, 1, InputCube[face].Data);
        }
        else
        {
          result = s.setImage2D(fmt, nvtt::Decoder_D3D9, InputCube[face].X, InputCube[face].Y, InputCube[face].Data);
        }
        if (!result || s.isNull() || !s.width() || !s.height())
        {
          Error = "Texture Processor: failed to create input cube surface";
          return false;
        }
        s.flipY();
      }
      surface = cube.unfold(nvtt::CubeLayout_Row);
      if (surface.isNull() || !surface.width() || !surface.height())
      {
        Error = "Texture Processor: failed to unfold the input cube surface";
        return false;
      }
    }
    else
    {
      LogI("Texture Processor: Setting surface image");
      bool result = false;
      if (fmt == nvtt::Format_Count)
      {
        result = surface.setImage(nvtt::InputFormat_BGRA_8UB, InputDataSizeX, InputDataSizeY, 1, InputData);
      }
      else
      {
        result = surface.setImage2D(fmt, nvtt::Decoder_D3D9, InputDataSizeX, InputDataSizeY, InputData);
      }
      if (!result || surface.isNull() || !surface.width() || !surface.height())
      {
        Error = "Texture Processor: failed to create input surface";
        return false;
      }
      surface.flipY();
    }
  }
  else if (InputFormat == TCFormat::G8)
  {
    if (InIsCube)
    {
      Error = "PF_G8 texture cube is not supported!";
      return false;
    }
    hasAlpha = false;
    // Don't use nvtt for G8. Feed InputData directly to the FreeImage
  }
  else if (InputFormat == TCFormat::G16)
  {
    if (InIsCube)
    {
      Error = "PF_G16 texture cube is not supported!";
      return false;
    }
    hasAlpha = false;
    // Don't use nvtt for G16. Feed InputData directly to the FreeImage
  }
  else
  {
    Error = "Texture Processor: unsupported input " + std::to_string((int)InputFormat);
    return false;
  }

  uint8* SeparateAlphaChannel = nullptr;
  FreeImageHolder holder(true);
  int bits = hasAlpha ? 32 : 24;
  if (InputFormat == TCFormat::G8)
  {
    bits = 8;
    holder.bmp = FreeImage_Allocate(InputDataSizeX, InputDataSizeY, bits);
    for (int y = 0; y < InputDataSizeY; ++y)
    {
      uint8* scanline = FreeImage_GetScanLine(holder.bmp, y);
      for (int x = 0; x < InputDataSizeX; ++x)
      {
        int32 offset = (InputDataSizeY - y - 1) * InputDataSizeX + x;
        scanline[x * sizeof(uint8) + 0] = *((uint8*)InputData + offset);
      }
    }
  }
  else if (InputFormat == TCFormat::G16)
  {
    bits = 16;
    holder.bmp = FreeImage_AllocateT(FIT_UINT16, InputDataSizeX, InputDataSizeY, bits);
    for (int y = 0; y < InputDataSizeY; ++y)
    {
      uint8* scanline = FreeImage_GetScanLine(holder.bmp, y);
      for (int x = 0; x < InputDataSizeX; ++x)
      {
        int32 offset = (InputDataSizeY - y - 1) * InputDataSizeX + x;
        scanline[x * sizeof(uint16) + 0] = *(((uint8*)InputData + offset * sizeof(uint16)) + 0);
        scanline[x * sizeof(uint16) + 1] = *(((uint8*)InputData + offset * sizeof(uint16)) + 1);
      }
    }
  }
  else
  {
    nvtt::CompressionOptions compressionOptions;
    compressionOptions.setFormat(hasAlpha ? nvtt::Format_RGBA : nvtt::Format_RGB);
    compressionOptions.setPixelFormat(hasAlpha ? 32 : 24, 0xFF0000, 0xFF00, 0xFF, hasAlpha ? 0xFF000000 : 0);
    if (Normal)
    {
      compressionOptions.setColorWeights(.4, .4, .2);
      surface.setNormalMap(Normal);
    }
    nvtt::OutputOptions outputOptions;
    TPOutputHandler ohandler;
    outputOptions.setSrgbFlag(SRGB);
    outputOptions.setOutputHandler(&ohandler);
    LogI("Texture Processor: Decompress DXT data");
    nvtt::Context ctx;

    try
    {
      if (!ctx.compress(surface, 0, 0, compressionOptions, outputOptions))
      {
        Error = "Texture Processor: Failed to decompress texture to RGBA!";
        return false;
      }
    }
    catch (...)
    {
      Error = "Texture Processor: NVTT failed to decompress texture to RGBA!";
      return false;
    }

    if (!ohandler.MipsCount)
    {
      Error = "Texture Processor: Failed to decompress DXT texture to RGBA!";
      return false;
    }

    if (!ohandler.Mips[0].Size)
    {
      Error = "Texture Processor: NVTT failed to decompress the texture!";
      return false;
    }

    if (hasAlpha && SplitAlpha)
    {
      uint8* SrcPtr = (uint8*)ohandler.Mips[0].Data;
      SeparateAlphaChannel = (uint8*)malloc(ohandler.Mips[0].SizeX * ohandler.Mips[0].SizeY);
      for (int x = 0; x < ohandler.Mips[0].SizeX; ++x)
      {
        for (int y = 0; y < ohandler.Mips[0].SizeY; ++y)
        {
          SeparateAlphaChannel[x * ohandler.Mips[0].SizeX + y] = SrcPtr[(x * ohandler.Mips[0].SizeX + y) * sizeof(uint32) + 3];
        }
      }
    }

    holder.bmp = FreeImage_Allocate(ohandler.Mips[0].SizeX, ohandler.Mips[0].SizeY, bits);
    memcpy(FreeImage_GetBits(holder.bmp), ohandler.Mips[0].Data, ohandler.Mips[0].Size);
  }
  holder.mem = FreeImage_OpenMemory();

  LogI("Texture Processor: Convert the buffer to FreeImage format");
  if (OutputFormat == TCFormat::TGA)
  {
    if (!FreeImage_SaveToMemory(FIF_TARGA, holder.bmp, holder.mem, 0))
    {
      Error = "Texture Processor: Failed to create a FreeImage(TARGA:" + std::to_string(bits) + ")";
      return false;
    }
  }
  else if (OutputFormat == TCFormat::PNG)
  {
    if (!FreeImage_SaveToMemory(FIF_PNG, holder.bmp, holder.mem, 0))
    {
      Error = "Texture Processor: Failed to create a FreeImage(PNG:" + std::to_string(bits) + ")";
      return false;
    }
  }
  else
  {
    Error = "Texture Processor: Unsupported IO combination BytesToFile(\"" + std::to_string((int)InputFormat) + "\"" + std::to_string((int)InputFormat) + ")";
    return false;
  }

  LogI("Texture Processor: Preparing to save the buffer");
  DWORD memBufferSize = 0;
  unsigned char* memBuffer = nullptr;
  if (!FreeImage_AcquireMemory(holder.mem, &memBuffer, &memBufferSize))
  {
    Error = "Texture Processor: Failed to acquire a memory buffer";
    return false;
  }

  if (!memBufferSize)
  {
    Error = "Texture Processor: FreeImageLib failed to acquire memory!";
    return false;
  }

  LogI("Saving data...");
  FWriteStream s(A2W(OutputPath));
  if (!s.IsGood())
  {
    Error = "Texture Processor: Failed to create a write stream to \"" + OutputPath + "\"";
    return false;
  }
  s.SerializeBytes(memBuffer, (FILE_OFFSET)memBufferSize);
  if (!s.IsGood())
  {
    Error = "Texture Processor: Failed to write data to the stream at: \"" + OutputPath + "\"";
    return false;
  }
  
  if (SeparateAlphaChannel)
  {
    FreeImageHolder holder(true);
    holder.bmp = FreeImage_Allocate(InputDataSizeX, InputDataSizeY, 8);
    for (int y = 0; y < InputDataSizeY; ++y)
    {
      uint8* scanline = FreeImage_GetScanLine(holder.bmp, y);
      for (int x = 0; x < InputDataSizeX; ++x)
      {
        int32 offset = (InputDataSizeY - y - 1) * InputDataSizeX + x;
        scanline[x * sizeof(uint8) + 0] = *((uint8*)SeparateAlphaChannel + offset);
      }
    }
    free(SeparateAlphaChannel);
    FreeImage_FlipVertical(holder.bmp);
    holder.mem = FreeImage_OpenMemory();
    if (OutputFormat == TCFormat::TGA)
    {
      FreeImage_SaveToMemory(FIF_TARGA, holder.bmp, holder.mem, 0);
    }
    else if (OutputFormat == TCFormat::PNG)
    {
      FreeImage_SaveToMemory(FIF_PNG, holder.bmp, holder.mem, 0);
    }
    DWORD memBufferSize = 0;
    unsigned char* memBuffer = nullptr;
    if (FreeImage_AcquireMemory(holder.mem, &memBuffer, &memBufferSize))
    {
      std::filesystem::path Dest = A2W(OutputPath);
      auto ext = Dest.extension();
      Dest.replace_extension();
      Dest += "_Alpha";
      Dest.replace_extension(ext);
      FWriteStream s(W2A(Dest.wstring()));
      s.SerializeBytes(memBuffer, (FILE_OFFSET)memBufferSize);
    }
  }

  return true;
}

bool TextureProcessor::BytesToDDS()
{
  if (InIsCube)
  {
    if (HasAVX2())
    {
      // Recompress the cube if we have AVX2. We want the output cube to have A8R8G8B8 format regardless of the input and output settings
      nvtt::CubeSurface cube;
      nvtt::Format fmt = nvtt::Format_Count;
      switch (InputFormat)
      {
      case TCFormat::DXT1:
        fmt = nvtt::Format_DXT1;
        break;
      case TCFormat::DXT3:
        fmt = nvtt::Format_DXT3;
        break;
      case TCFormat::DXT5:
        fmt = nvtt::Format_DXT5;
        break;
      case TCFormat::ARGB8:
        break;
      default:
        Error = "Texture Processor: cube's pixel format is not supported!";
        return false;
      }

      for (int32 faceIdx = 0; faceIdx < InputCube.size(); ++faceIdx)
      {
        nvtt::Surface& s = cube.face(faceIdx);
        bool result = false;
        if (fmt == nvtt::Format_Count)
        {
          result = s.setImage(nvtt::InputFormat_BGRA_8UB, InputCube[faceIdx].X, InputCube[faceIdx].Y, 1, InputCube[faceIdx].Data);
        }
        else
        {
          result = s.setImage2D(fmt, nvtt::Decoder_D3D10, InputCube[faceIdx].X, InputCube[faceIdx].Y, InputCube[faceIdx].Data);
        }
        if (!result)
        {
          Error = "Texture Processor: failed to set cube surface!";
          return false;
        }
      }

      nvtt::CompressionOptions compressionOptions;
      compressionOptions.setFormat(nvtt::Format_RGBA);
      compressionOptions.setPixelFormat(32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);

      nvtt::OutputOptions outputOptions;
      TPOutputHandler ohandler;
      outputOptions.setSrgbFlag(SRGB);
      outputOptions.setOutputHandler(&ohandler);

      nvtt::Context ctx;
      if (!ctx.compress(cube, 1, compressionOptions, outputOptions))
      {
        Error = "Texture Processor: Failed to compress cube surface!";
        return false;
      }

      DDS::DDSHeader header;
      header.D3D9.dwWidth = InputCube[0].X;
      header.D3D9.dwHeight = InputCube[0].Y;
      header.D3D9.dwCaps = DDS::DDSCAPS::DDSCAPS_COMPLEX | DDS::DDSCAPS::DDSCAPS_TEXTURE | DDS::DDSCAPS::DDSCAPS_MIPMAP;
      header.D3D9.dwCaps2 = DDS::DDSCAPS2::DDSCAPS2_CUBEMAP | DDS::DDSCAPS2::DDSCAPS2_CUBEMAP_ALLFACES;
      header.D3D9.dwFlags &= ~DDS::DDSD::DDSD_LINEARSIZE;
      header.D3D9.dwFlags |= DDS::DDSD::DDSD_PITCH;
      header.D3D9.ddspf.dwFourCC = DDS::FCC::FCC_DX10;
      header.D3D9.ddspf.dwFlags = DDS::DDPF_FOURCC;
      header.D3D10.dxgiFormat = DDS::DXGI_FORMAT_B8G8R8A8_UNORM;
      header.D3D10.miscFlag = DDS::D3D10_MISC_FLAG::DDS_MISC_TEXTURECUBE;
      header.D3D9.dwPitchOrLinearSize = (header.D3D9.dwWidth * 32 + 7) / 8;

      FWriteStream s(A2W(OutputPath));
      s << header;
      for (int32 faceIdx = 0; faceIdx < ohandler.MipsCount; ++faceIdx)
      {
        s.SerializeBytes(ohandler.Mips[faceIdx].Data, ohandler.Mips[faceIdx].Size);
      }

      if (!s.IsGood())
      {
        Error = "Texture Processor: Failed to write texture to disk!";
      }
      return s.IsGood();
    }

    // We can't recompress faces without nvtt, so let's use existing face compression

    DDS::DDSHeader header;
    header.D3D9.dwWidth = InputCube[0].X;
    header.D3D9.dwHeight = InputCube[0].Y;
    header.D3D9.dwCaps = DDS::DDSCAPS::DDSCAPS_COMPLEX | DDS::DDSCAPS::DDSCAPS_TEXTURE | DDS::DDSCAPS::DDSCAPS_MIPMAP;
    header.D3D9.dwCaps2 = DDS::DDSCAPS2::DDSCAPS2_CUBEMAP | DDS::DDSCAPS2::DDSCAPS2_CUBEMAP_ALLFACES;
    header.D3D9.ddspf.dwFourCC = DDS::FCC::FCC_DX10;
    header.D3D9.ddspf.dwFlags = DDS::DDPF_FOURCC;
    header.D3D10.miscFlag = DDS::D3D10_MISC_FLAG::DDS_MISC_TEXTURECUBE;

    switch (InputFormat)
    {
    case TCFormat::DXT1:
      header.D3D9.dwPitchOrLinearSize = header.CalculateMipmapSize();
      header.D3D10.dxgiFormat = DDS::DXGI_FORMAT_BC1_TYPELESS;
      break;
    case TCFormat::DXT3:
      header.D3D9.dwPitchOrLinearSize = header.CalculateMipmapSize();
      header.D3D10.dxgiFormat = DDS::DXGI_FORMAT_BC2_TYPELESS;
      break;
    case TCFormat::DXT5:
      header.D3D9.dwPitchOrLinearSize = header.CalculateMipmapSize();
      header.D3D10.dxgiFormat = DDS::DXGI_FORMAT_BC3_TYPELESS;
      break;
    case TCFormat::ARGB8:
      header.D3D9.dwFlags &= ~DDS::DDSD::DDSD_LINEARSIZE;
      header.D3D9.dwFlags |= DDS::DDSD::DDSD_PITCH;
      header.D3D10.dxgiFormat = DDS::DXGI_FORMAT_B8G8R8A8_UNORM;
      header.D3D9.dwPitchOrLinearSize = (header.D3D9.dwWidth * 32 + 7) / 8;
      break;
    default:
      Error = "Texture Processor: cube's pixel format is not supported!";
      return false;
    }

    FWriteStream s(A2W(OutputPath));
    s << header;
    for (int32 faceIdx = 0; faceIdx < InputCube.size(); ++faceIdx)
    {
      s.SerializeBytes(InputCube[faceIdx].Data, InputCube[faceIdx].Size);
    }
    if (!s.IsGood())
    {
      Error = "Texture Processor: Failed to write texture to disk!";
    }
    return s.IsGood();
  }
  DDS::DDSHeader header;
  header.D3D9.dwWidth = InputDataSizeX;
  header.D3D9.dwHeight = InputDataSizeY;
  switch (InputFormat)
  {
  case TextureProcessor::TCFormat::DXT1:
    header.D3D10.dxgiFormat = DDS::DXGI_FORMAT_BC1_TYPELESS;
    break;
  case TextureProcessor::TCFormat::DXT3:
    header.D3D10.dxgiFormat = DDS::DXGI_FORMAT_BC2_TYPELESS;
    break;
  case TextureProcessor::TCFormat::DXT5:
    header.D3D10.dxgiFormat = DDS::DXGI_FORMAT_BC3_TYPELESS;
    break;
  case TextureProcessor::TCFormat::ARGB8:
    header.D3D9.ddspf.dwFlags = DDS::DDPF_RGB | DDS::DDPF_ALPHAPIXELS;
    header.D3D9.ddspf.dwFourCC = 0;
    header.D3D9.ddspf.dwRGBBitCount = 32;
    header.D3D9.ddspf.dwRBitMask = 0x00ff0000;
    header.D3D9.ddspf.dwGBitMask = 0x0000ff00;
    header.D3D9.ddspf.dwBBitMask = 0x000000ff;
    header.D3D9.ddspf.dwABitMask = 0xff000000;
    break;
  case TextureProcessor::TCFormat::G8:
    header.D3D9.ddspf.dwFlags = DDS::DDPF_LUMINANCE;
    header.D3D9.ddspf.dwFourCC = 0;
    header.D3D9.ddspf.dwRGBBitCount = 8;
    header.D3D9.ddspf.dwRBitMask = 0x000000ff;
    header.D3D9.ddspf.dwGBitMask = 0x000000ff;
    header.D3D9.ddspf.dwBBitMask = 0x000000ff;
    break;
  case TextureProcessor::TCFormat::G16:
    header.D3D9.ddspf.dwFlags = DDS::DDPF_LUMINANCE;
    header.D3D9.ddspf.dwFourCC = 0;
    header.D3D9.ddspf.dwRGBBitCount = 16;
    header.D3D9.ddspf.dwRBitMask = 0x0000ffff;
    header.D3D9.ddspf.dwGBitMask = 0x0000ffff;
    header.D3D9.ddspf.dwBBitMask = 0x0000ffff;
    break;
  default:
    Error = "Texture Processor: pixel format is not supported!";
    return false;
  }
  header.D3D9.dwPitchOrLinearSize = header.CalculateMipmapSize();
  FWriteStream s(A2W(OutputPath));
  s << header;
  s.SerializeBytes(InputData, InputDataSize);
  if (!s.IsGood())
  {
    Error = "Texture Processor: failed to save the file!";
    return false;
  }
  return true;
}

bool TextureProcessor::BytesToBytes()
{
  Error = "Texture Processor: B2B conversion unsupported!";
  return false;
}

bool TextureProcessor::FileToBytes()
{
  FreeImageHolder holder(true);
  FREE_IMAGE_FORMAT fmt;
  if (InputFormat == TCFormat::PNG)
  {
    fmt = FIF_PNG;
  }
  else if (InputFormat == TCFormat::TGA)
  {
    fmt = FIF_TARGA;
  }
  else if (InputFormat == TCFormat::DDS)
  {
    return DDSToBytes();
  }
  else
  {
    Error = "Texture Processor: Input format \"" + std::to_string((int)InputFormat) + "\" is not supported!";
    return false;
  }

  if (!HasAVX2())
  {
    Error = "Texture Processor: Your CPU does not support AVX2 instructions. Please, use DDS format to import the texture.";
    return false;
  }
  
  holder.bmp = FreeImage_LoadU(fmt, A2W(InputPath).c_str());
  if (FreeImage_GetBPP(holder.bmp) != 32)
  {
    holder.bmp = FreeImage_ConvertTo32Bits(holder.bmp);
  }

  Alpha = FreeImage_IsTransparent(holder.bmp);
  uint8* data = (uint8*)FreeImage_GetBits(holder.bmp);

  if (!data)
  {
    Error = "Texture Processor: FreeImage failed to get bitmap!";
    FreeImage_Unload(holder.bmp);
    return false;
  }

  nvtt::Surface surface;
  surface.setWrapMode(AddressModeToNvtt(AddressX));
  nvtt::Format outFmt = nvtt::Format_Count;

  if (OutputFormat == TCFormat::DXT1)
  {
    outFmt = Alpha ? nvtt::Format_DXT1a : nvtt::Format_DXT1;
  }
  else if (OutputFormat == TCFormat::DXT3)
  {
    outFmt = nvtt::Format_DXT3;
  }
  else if (OutputFormat == TCFormat::DXT5)
  {
    outFmt = nvtt::Format_DXT5;
  }
  else if (OutputFormat == TCFormat::ARGB8)
  {
    outFmt = nvtt::Format_RGBA;
  }
  else if (OutputFormat == TCFormat::G8)
  {
    outFmt = nvtt::Format_RGBA;
    Alpha = false;
  }

  try
  {
    if (!surface.setImage(nvtt::InputFormat_BGRA_8UB, FreeImage_GetWidth(holder.bmp), FreeImage_GetHeight(holder.bmp), 1, data))
    {
      Error = "Texture Processor: failed to set a surface.";
      FreeImage_Unload(holder.bmp);
      return false;
    }
  }
  catch (...)
  {
    Error = "Texture Processor: NVTT failed to set a surface.";
    FreeImage_Unload(holder.bmp);
    return false;
  }
  if (Normal)
  {
    surface.setNormalMap(true);
  }
  surface.setAlphaMode(Alpha ? nvtt::AlphaMode_Transparency : nvtt::AlphaMode_None);
  surface.flipY();
  

  nvtt::CompressionOptions compressionOptions;
  compressionOptions.setFormat(outFmt);
  compressionOptions.setQuality(nvtt::Quality_Production);
  if (Normal)
  {
    compressionOptions.setColorWeights(.4, .4, .2);
  }
  if (outFmt == nvtt::Format_DXT3)
  {
    compressionOptions.setQuantization(false, true, false);
  }
  else if (outFmt == nvtt::Format_DXT1a)
  {
    compressionOptions.setQuantization(false, true, true, 127);
  }
  else if (outFmt == nvtt::Format_RGBA)
  {
    if (OutputFormat == TCFormat::G8)
    {
      compressionOptions.setPixelFormat(8, 0xff, 0, 0, 0);
    }
    else
    {
      compressionOptions.setPixelFormat(32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    }
  }

  TPOutputHandler ohandler;
  nvtt::OutputOptions outputOptions;
  outputOptions.setOutputHandler(&ohandler);
  nvtt::Context context;
  
  int32 idx = 0;
  int32 sizeX = FreeImage_GetWidth(holder.bmp);
  int32 sizeY = FreeImage_GetHeight(holder.bmp);

  const int32 minX = 4; // TODO: replace with PF block width & height
  const int32 minY = minX;
  OutputDataSize = 0;

  if (sizeX < minX || sizeY < minY)
  {
    Error = "Texture Processor: failed to compress the bitmap. The image is too small!";
    return false;
  }

  while (idx < TPOutputHandler::MaxMipCount)
  {
    if (!context.compress(surface, 0, idx, compressionOptions, outputOptions))
    {
      Error = "Texture Processor: failed to compress the bitmap.";
      return false;
    }

    OutputDataSize += ohandler.Mips[idx].Size;

    if (GenerateMips)
    {
      if (!surface.buildNextMipmap(MipFilterTypeToNvtt(MipFilter)))
      {
        break;
      }
      sizeX = surface.width();
      sizeY = surface.height();
      idx++;
    }
    else
    {
      break;
    }
  }

  if (!(OutputMipCount = ohandler.MipsCount))
  {
    Error = "Texture Processor: failed to compress the bitmap. No mipmaps were generated.";
    return false;
  }
  if (!OutputDataSize)
  {
    Error = "Texture Processor: NVTT encoding error!";
    return false;
  }

  OutputData = malloc(OutputDataSize);
  
  int32 offset = 0;
  for (int32 idx = 0; idx < OutputMipCount; ++idx)
  {
    TPOutputHandler::Mip& mip = ohandler.Mips[idx];
    memcpy((uint8*)OutputData + offset, mip.Data, mip.Size);
    OutputMips.push_back({mip.SizeX, mip.SizeY, mip.Size, (uint8*)OutputData + offset });
    offset += ohandler.Mips[idx].Size;
  }
  return true;
}

bool TextureProcessor::DDSToBytes()
{
  FReadStream s = FReadStream(A2W(InputPath));
  if (!s.IsGood())
  {
    Error = "Texture Processor: Failed to open the texture file!";
    return false;
  }
  
  DDS::DDSHeader header;
  try
  {
    s << header;
  }
  catch (const std::exception& e)
  {
    Error = e.what();
    return false;
  }
  EPixelFormat fmt = header.GetPixelFormat();
  if (fmt == PF_Unknown)
  {
    switch (header.D3D10.dxgiFormat)
    {
    case DDS::DXGI_FORMAT_BC7_TYPELESS:
    case DDS::DXGI_FORMAT_BC7_UNORM:
    case DDS::DXGI_FORMAT_BC7_UNORM_SRGB:
      // BC7 is a default format in NVTE. Show more specific error to make sure that user understands what's wrong.
      Error = "Texture Processor: The input file has BC7 format. Tera does not support it!\nPlease, use BC1(DXT1) or BC3(DXT5) format when saving your DDS file.";
      return false;
    }
    Error = "Texture Processor: The input file has unsupported pixel format.\nPlease, use BC1(DXT1) or BC3(DXT5) format when saving your DDS file.";
    return false;
  }
  else if (fmt == PF_DXT1)
  {
    OutputFormat = TCFormat::DXT1;
  }
  else if (fmt == PF_DXT3)
  {
    OutputFormat = TCFormat::DXT3;
  }
  else if (fmt == PF_DXT5)
  {
    OutputFormat = TCFormat::DXT5;
    Alpha = true;
  }
  else if (fmt == PF_A8R8G8B8)
  {
    OutputFormat = TCFormat::ARGB8;
    Alpha = true;
  }
  else if (fmt == PF_G8)
  {
    OutputFormat = TCFormat::G8;
    Alpha = false;
  }
  FILE_OFFSET mipSize = header.CalculateMipmapSize();
  if (!mipSize)
  {
    Error = "Texture Processor: Failed to calculate texture size.";
    return false;
  }

  OutputData = malloc(mipSize);
  s.SerializeBytes(OutputData, mipSize);
  OutputDataSize = mipSize;
  OutputMipCount = 1;
  OutputMips.clear();
  OutputMips.push_back({header.GetWidth(), header.GetHeight(), OutputDataSize, OutputData});
  return true;
}
