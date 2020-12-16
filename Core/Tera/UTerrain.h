#pragma once
#include "UObject.h"
#include "UActor.h"
#include "UTexture.h"
#include "UMaterial.h"

#include <array>

struct FTerrainHeight {
  FTerrainHeight() = default;
  FTerrainHeight(uint16 v)
    : Value(v)
  {}

  friend FStream& operator<<(FStream& s, FTerrainHeight& h)
  {
    return s << h.Value;
  }

  uint16 Value = 0;
};

struct FTerrainInfoData
{
  uint8 Data = 0;

  enum InfoFlags
  {
    TID_Visibility_Off = 0x0001,
  };

  FTerrainInfoData() = default;
  FTerrainInfoData(uint8 d) :
    Data(d)
  {
  }

  friend FStream& operator<<(FStream& s, FTerrainInfoData& H)
  {
    return s << H.Data;
  }

  inline bool IsVisible() const
  {
    return ((Data & TID_Visibility_Off) == 0);
  }
};

struct FAlphaMap {
  std::vector<uint8> Data;

  friend FStream& operator<<(FStream& s, FAlphaMap& m)
  {
    return s << m.Data;
  }
};

class UTerrain : public UActor {
public:
  DECL_UOBJ(UTerrain, UActor);
  UTerrain(FObjectExport* exp);

  UPROP(int32, MaxTesselationLevel, 0);
  UPROP(int32, NumPatchesX, 0);
  UPROP(int32, NumPatchesY, 0);
  UPROP(int32, NumVerticesX, 0);
  UPROP(int32, NumVerticesY, 0);
  UPROP(int32, NumSectionsX, 0);
  UPROP(int32, NumSectionsY, 0);

  bool RegisterProperty(FPropertyTag* property) override;
  void Serialize(FStream& s) override;
  void PostLoad() override;
  
  void GetHeightMap(uint16*& result, int32& width, int32& height, bool resample = false);
  void GetVisibilityMap(uint8*& result, int32& width, int32& height, bool resample = false);

  inline bool HasVisibilityData()
  {
    return HasTransparency;
  }

protected:

protected:
  std::vector<FTerrainHeight> Heights;
  std::vector<FTerrainInfoData> InfoData;
  int32 Unk1 = 0;
  std::vector<FAlphaMap> AlphaMaps;
  std::vector<PACKAGE_INDEX> WeightMapTextureIndices;
  std::vector<UTerrainWeightMapTexture*> WeightMapTextures;
  void* MaterialData = nullptr;
  FILE_OFFSET MaterialDataSize = 0;
  bool HasTransparency = false;
};