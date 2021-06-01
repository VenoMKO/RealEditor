#pragma once
#include <Tera/Core.h>
#include <Tera/FString.h>
#include <array>
#include <filesystem>

class TextureProcessor {
public:
  enum class TCFormat {
    None = 0,
    DXT,
    DXT1, 
    DXT3,
    DXT5,
    ARGB8,
    G8,
    G16,
    PNG,
    TGA,
    DDS
  };

  static TCFormat GetTcFormatByExtension(const FString& ext)
  {
    if (ext == "dds" || ext == ".dds")
    {
      return TCFormat::DDS;
    }
    if (ext == "tga" || ext == ".tga")
    {
      return TCFormat::TGA;
    }
    if (ext == "png" || ext == ".png")
    {
      return TCFormat::PNG;
    }
    return HasAVX2() ? TCFormat::TGA : TCFormat::DDS;
  }

  TextureProcessor(TCFormat from, TCFormat to)
    : InputFormat(from)
    , OutputFormat(to)
  {}

  ~TextureProcessor()
  {
    free(InputData);
    free(OutputData);
    for (auto& cube : InputCube)
    {
      free(cube.Data);
    }
  }

  inline void SetInputCubeFace(int32 faceIdx, void* data, int32 size, int32 width, int32 height)
  {
    InIsCube = data;
    if (InputCube[faceIdx].Data)
    {
      free(InputCube[faceIdx].Data);
      InputCube[faceIdx].Data = nullptr;
    }
    if (size && data)
    {
      InputCube[faceIdx].Data = malloc(size);
      memcpy(InputCube[faceIdx].Data, data, size);
    }
    InputCube[faceIdx].Size = size;
    InputCube[faceIdx].X = width;
    InputCube[faceIdx].Y = height;
  }

  inline void SetInputData(void* data, int32 size)
  {
    if (InputData)
    {
      free(InputData);
      InputData = nullptr;
    }
    if (size && data)
    {
      InputData = malloc(size);
      memcpy(InputData, data, size);
    }
    InputDataSize = size;
  }

  inline void SetInputDataDimensions(int32 sizeX, int32 sizeY)
  {
    InputDataSizeX = sizeX;
    InputDataSizeY = sizeY;
  }

  inline void SetInputPath(const std::string& inputPath)
  {
    InputPath = inputPath;
  }

  inline void SetOutputPath(const std::string& outputPath)
  {
    static const std::vector<wchar> forbiddenChars = { L'\\', L'/', L':', L'*', L'?', L'\"', L'<', L'>', L'|'};
    std::filesystem::path tmp(A2W(outputPath));
    std::wstring name = tmp.filename();
    for (size_t pos = 0; pos < name.size(); ++pos)
    {
      if (std::find(forbiddenChars.begin(), forbiddenChars.end(), name[pos]) != forbiddenChars.end())
      {
        name[pos] = L'_';
      }
    }
    tmp.replace_filename(name);
    OutputPath = W2A(tmp.wstring());
  }

  inline std::string GetError() const
  {
    return Error;
  }

  inline void SetSrgb(bool srgb)
  {
    SRGB = srgb;
  }

  inline void SetNormal(bool normal)
  {
    Normal = normal;
  }

  inline void SetAddressX(TextureAddress x)
  {
    AddressX = x;
  }

  inline void SetAddressY(TextureAddress y)
  {
    AddressY = y;
  }

  inline void SetGenerateMips(bool generate)
  {
    GenerateMips = generate;
  }

  inline void SetMipFilter(MipFilterType filter)
  {
    MipFilter = filter;
  }

  inline bool GetAlpha() const
  {
    return Alpha;
  }

  inline void SetOutputFormat(TCFormat to)
  {
    OutputFormat = to;
  }

  inline void SetSplitAlpha(bool flag)
  {
    SplitAlpha = flag;
  }

  inline void ClearOutput()
  {
    if (OutputData)
    {
      free(OutputData);
    }
    OutputMips.clear();
  }

  bool Process();

  struct OutputMip {
    int32 SizeX = 0;
    int32 SizeY = 0;
    int32 Size = 0;
    void* Data = nullptr;
  };

  struct Cube {
    void* Data = nullptr;
    int32 Size = 0;
    int32 X = 0;
    int32 Y = 0;
  };

  inline const std::vector<OutputMip>& GetOutputMips() const
  {
    return OutputMips;
  }

  TCFormat GetOutputFormat() const
  {
    return OutputFormat;
  }

  bool UnfoldCube();

private:
  bool BytesToFile();
  bool BytesToDDS();
  bool BytesToBytes();
  bool FileToBytes();
  bool DDSToBytes();

  inline bool InputCubeIsValid() const
  {
    int32 x = InputCube[0].X;
    int32 y = InputCube[0].Y;
    for (const Cube& c : InputCube)
    {
      if (!c.Data || !c.Size || x != c.X || y != c.Y || !x || !y)
      {
        return false;
      }
    }
    return true;
  }

private:
  TCFormat InputFormat = TCFormat::None;
  TCFormat OutputFormat = TCFormat::None;

  void* InputData = nullptr;
  int32 InputDataSize = 0;
  int32 InputDataSizeX = 0;
  int32 InputDataSizeY = 0;
  std::string InputPath;
  std::array<Cube, 6> InputCube;

  void* OutputData = nullptr;
  int32 OutputDataSize = 0;
  int32 OutputMipCount = 0;

  std::vector<OutputMip> OutputMips;

  std::string OutputPath;
  
  bool SRGB = false;
  bool Alpha = false;
  bool Normal = false;
  bool GenerateMips = false;
  bool InIsCube = false;
  bool SplitAlpha = false;

  MipFilterType MipFilter = MipFilterType::Mitchell;

  TextureAddress AddressX = TA_Wrap;
  TextureAddress AddressY = TA_Wrap;

  std::string Error;
};