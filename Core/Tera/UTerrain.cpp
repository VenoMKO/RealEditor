#include "UTerrain.h"
#include "FPackage.h"
#include "Cast.h"

template <typename T>
T* ResampleData(const T* input, int32 width, int32 height, int32 newWidth, int32 newHeight)
{
  T* result = (T*)calloc(newWidth * newHeight, sizeof(T));

  double scaleX = (double)(width - 1) / (newWidth - 1);
  double scaleY = (double)(height - 1) / (newHeight - 1);

  for (int32 newY = 0; newY < newHeight; ++newY)
  {
    for (int32 newX = 0; newX < newWidth; ++newX)
    {
      double x = newX * scaleX;
      double y = newY * scaleY;

      int32 x0 = floor(x);
      int32 x1 = std::min<int32>(floor(x + 1.), width - 1);
      int32 y0 = floor(y);
      int32 y1 = std::min<int32>(floor(y + 1.), height - 1);

      T a = input[y0 * width + x0];
      T b = input[y0 * width + x1];
      T c = input[y1 * width + x0];
      T d = input[y1 * width + x1];

      result[newY * newWidth + newX] = (T)Bilerp(a, b, c, d, x - floor(x), y - floor(y));
    }
  }
  return result;
}

UTerrain::UTerrain(FObjectExport* exp)
  : Super(exp)
{
  DrawScale3D = FVector(256, 256, 256);
}

bool UTerrain::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, MaxTesselationLevel))
  {
    MaxTesselationLevelProperty = property;
    MaxTesselationLevel = property->Value->GetInt();
    return true;
  }
  if (PROP_IS(property, NumPatchesX))
  {
    NumPatchesXProperty = property;
    NumPatchesX = property->Value->GetInt();
    return true;
  }
  if (PROP_IS(property, NumPatchesY))
  {
    NumPatchesYProperty = property;
    NumPatchesY = property->Value->GetInt();
    return true;
  }
  if (PROP_IS(property, NumVerticesX))
  {
    NumVerticesXProperty = property;
    NumVerticesX = property->Value->GetInt();
    return true;
  }
  if (PROP_IS(property, NumVerticesY))
  {
    NumVerticesYProperty = property;
    NumVerticesY = property->Value->GetInt();
    return true;
  }
  if (PROP_IS(property, NumSectionsX))
  {
    NumSectionsXProperty = property;
    NumSectionsX = property->Value->GetInt();
    return true;
  }
  if (PROP_IS(property, NumSectionsY))
  {
    NumSectionsYProperty = property;
    NumSectionsY = property->Value->GetInt();
    return true;
  }
  return false;
}

void UTerrain::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Heights;
  s << InfoData;
  s << Unk1;
  s << AlphaMaps;
  s << WeightMapTextureIndices;
  // Cached materials and compiled shaders
  // TODO: serialize
  SerializeTrailingData(s);
}

void UTerrain::PostLoad()
{
  Super::PostLoad();
  for (PACKAGE_INDEX idx : WeightMapTextureIndices)
  {
    WeightMapTextures.push_back(Cast<UTerrainWeightMapTexture>(GetPackage()->GetObject(idx)));
  }
  for (int32 idx = 0; idx < InfoData.size(); ++idx)
  {
    if (!InfoData[idx].IsVisible())
    {
      HasTransparency = true;
      break;
    }
  }
}

void UTerrain::GetHeightMap(uint16*& result, int32& width, int32& height, bool resample)
{
  result = nullptr;
  width = NumVerticesX;
  height = NumVerticesY;

  uint16* heights = (uint16*)calloc(width * height, sizeof(uint16));
  for (int32 idx = 0; idx < Heights.size(); ++idx)
  {
    heights[idx] = Heights[idx].Value;
  }
  if (resample && WeightMapTextures.size())
  {
    int32 newWidth = WeightMapTextures.front()->SizeX;
    int32 newHeight = WeightMapTextures.front()->SizeY;
    if (newWidth != width || newHeight != height)
    {
      result = ResampleData(heights, width, height, newWidth, newHeight);
      width = newWidth;
      height = newHeight;
      free(heights);
      return;
    }
  }
  result = heights;
}

void UTerrain::GetVisibilityMap(uint8*& result, int32& width, int32& height, bool resample)
{
  result = nullptr;
  width = NumVerticesX;
  height = NumVerticesY;

  uint8* mask = (uint8*)calloc(width * height, sizeof(uint8));
  for (int32 idx = 0; idx < InfoData.size(); ++idx)
  {
    mask[idx] = InfoData[idx].IsVisible() ? 0x00 : 0xFF;
  }
  if (resample && WeightMapTextures.size())
  {
    int32 newWidth = WeightMapTextures.front()->SizeX;
    int32 newHeight = WeightMapTextures.front()->SizeY;
    if (newWidth != width || newHeight != height)
    {
      result = ResampleData(mask, width, height, newWidth, newHeight);
      width = newWidth;
      height = newHeight;
      free(mask);
      return;
    }
  }
  result = mask;
}
