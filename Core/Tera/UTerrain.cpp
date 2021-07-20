#include "UTerrain.h"
#include "FPackage.h"
#include "Cast.h"

#include <Utils/ALog.h>

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
  SUPER_REGISTER_PROP();
  REGISTER_INT_PROP(MaxTesselationLevel);
  REGISTER_INT_PROP(NumPatchesX);
  REGISTER_INT_PROP(NumPatchesY);
  REGISTER_INT_PROP(NumVerticesX);
  REGISTER_INT_PROP(NumVerticesY);
  REGISTER_INT_PROP(NumSectionsX);
  REGISTER_INT_PROP(NumSectionsY);
  if (PROP_IS(property, Layers))
  {
    LayersProperty = property;
    Layers.clear();
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
  s << WeightMapTextures;
  // Cached materials and compiled shaders
  // TODO: serialize
  SerializeTrailingData(s);
}

void UTerrain::PostLoad()
{
  Super::PostLoad();
  for (int32 idx = 0; idx < InfoData.size(); ++idx)
  {
    if (!InfoData[idx].IsVisible())
    {
      HasTransparency = true;
      break;
    }
  }
  if (LayersProperty)
  {
    for (FPropertyValue* structValue : LayersProperty->GetArray())
    {
      FTerrainLayer layer;
      for (FPropertyValue* tagValue : structValue->GetArray())
      {
        FPropertyTag* tag = tagValue->GetPropertyTagPtr();
        if (tag->Name == "Name")
        {
          layer.Name = tag->GetString();
          continue;
        }
        if (tag->Name == "Setup")
        {
          layer.Setup = Cast<UTerrainLayerSetup>(tag->GetObjectValuePtr());
          continue;
        }
        if (tag->Name == "AlphaMapIndex")
        {
          layer.AlphaMapIndex = tag->GetInt();
          continue;
        }
        if (tag->Name == "Hidden")
        {
          layer.Hidden = tag->GetBool();
          continue;
        }
      }
      Layers.push_back(layer);
    }
    for (const FTerrainLayer& layer : Layers)
    {
      LoadObject(layer.Setup);
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
  for (int32 y = 0; y < NumVerticesY; ++y)
  {
    for (int32 x = 0; x < NumVerticesX; ++x)
    {
      mask[y * NumVerticesX + x] = InfoData[std::clamp(y, 0, NumPatchesY - 1) * NumVerticesX + std::clamp(x, 0, NumPatchesX - 1)].IsVisible() ? 0x00 : 0xFF;
    }
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

float UTerrain::GetHeightMapRatioX()
{
  for (UTexture2D* weightmap : WeightMapTextures)
  {
    if (!weightmap || !weightmap->SizeX)
    {
      continue;
    }
    return (float)NumPatchesX / float(weightmap->SizeX - 1);
  }
  return 1.f;
}

float UTerrain::GetHeightMapRatioY()
{
  for (UTexture2D* weightmap : WeightMapTextures)
  {
    if (!weightmap || !weightmap->SizeY)
    {
      continue;
    }
    return (float)NumPatchesY / float(weightmap->SizeY - 1);
  }
  return 1.f;
}

bool UTerrain::GetWeightMapChannel(int32 idx, void*& data, int32& width, int32& height)
{
  uint32 len = WeightMapTextures.front()->SizeX * WeightMapTextures.front()->SizeY;
  if (uint8* channel = (uint8*)malloc(len))
  {
    memcpy(channel, &AlphaMaps[idx].Data[0], len);
    data = (void*)channel;
    width = WeightMapTextures.front()->SizeX;
    height = WeightMapTextures.front()->SizeY;
  }
  return data != nullptr;
}

std::vector<UTerrainWeightMapTexture*> UTerrain::GetWeightMaps()
{
  return WeightMapTextures.GetObjects();
}

bool UTerrainMaterial::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(MappingScale);
  REGISTER_FLOAT_PROP(MappingRotation);
  REGISTER_FLOAT_PROP(MappingPanU);
  REGISTER_FLOAT_PROP(MappingPanV);
  REGISTER_TOBJ_PROP(Material, UMaterialInterface*);
  REGISTER_TOBJ_PROP(DisplacementMap, UTexture2D*);
  REGISTER_FLOAT_PROP(DisplacementScale);
  return false;
}

void UTerrainMaterial::PostLoad()
{
  Super::PostLoad();
  LoadObject(Material);
  LoadObject(DisplacementMap);
}

bool UTerrainLayerSetup::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, Materials))
  {
    MaterialsProperty = property;
    Materials.clear();
    return true;
  }
  return false;
}

void UTerrainLayerSetup::PostLoad()
{
  Super::PostLoad();
  if (MaterialsProperty)
  {
    for (FPropertyValue* structValue : MaterialsProperty->GetArray())
    {
      FTerrainFilteredMaterial mat;
      bool found = false;
      for (FPropertyValue* tagValue : structValue->GetArray())
      {
        FPropertyTag* tag = tagValue->GetPropertyTagPtr();
        if (tag->Name == "Material")
        {
          mat.Material = Cast<UTerrainMaterial>(tag->GetObjectValuePtr());
          if (found)
          {
            break;
          }
          found = true;
          continue;
        }
        if (tag->Name == "Alpha")
        {
          mat.Alpha = tag->GetFloat();
          if (found)
          {
            break;
          }
          found = true;
          continue;
        }
      }
      Materials.push_back(mat);
    }
    for (const FTerrainFilteredMaterial& mat : Materials)
    {
      LoadObject(mat.Material);
    }
  }
}
