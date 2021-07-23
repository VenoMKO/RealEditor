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

struct FTerrainMaterialMask {
  uint64 BitMask = 0;
  uint32 NumBits = 0;

  FTerrainMaterialMask() = default;

  FTerrainMaterialMask(uint32 numBits)
    :	BitMask(0)
    ,	NumBits(numBits)
  {}

  bool Get(uint32 index) const 
  { 
    return BitMask & (((uint64)1) << (index & 63));
  }
  
  void Set(uint32 index, bool v)
  { 
    if(v) 
    {
      BitMask |= ((uint64)1) << (index & 63);
    }
    else
    {
      BitMask &= ~(((uint64)1) << (index & 63));
    }
  }
  
  int32 Num() const
  {
    return NumBits;
  }

  bool operator==(const FTerrainMaterialMask& a) const
  {
    return (NumBits == a.NumBits) && (BitMask == a.BitMask);
  }

  friend FStream& operator<<(FStream& s, FTerrainMaterialMask& m);
};

class FTerrainMaterialResource : public FMaterial {
public:
  ~FTerrainMaterialResource()
  {
    for (auto* entry : ShaderCache)
    {
      delete entry;
    }
    for (auto* entry : ShaderCache2)
    {
      delete entry;
    }
  }

  friend FStream& operator<<(FStream& s, FTerrainMaterialResource& r);
  
  std::vector<FShaderCacheEntry*> ShaderCache;
  std::vector<FShaderCacheEntry2*> ShaderCache2;
  DECL_UREF(class UTerrain, Terrain);
  FTerrainMaterialMask Mask;
  std::vector<FGuid> MaterialIds;
  FStaticParameterSet StaticParameters;
  uint32 Unk1 = 0;
  uint32 Unk2 = 1;
  FName Unk3;
  FGuid Unk4;
  FString Unk5;
  FGuid Unk6;
  uint32 Unk7[3] = { 0, 0, 0 };
  FGuid LightmassGuid;
};

class UTerrainMaterial : public UObject {
public:
  DECL_UOBJ(UTerrainMaterial, UObject);

  UPROP(float, MappingScale, 4.f);
  UPROP(float, MappingRotation, 0.f);
  UPROP(float, MappingPanU, 0.f);
  UPROP(float, MappingPanV, 0.f);
  UPROP(UMaterialInterface*, Material, nullptr);
  UPROP(UTexture2D*, DisplacementMap, nullptr);
  UPROP(float, DisplacementScale, 1.f);

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};

struct FTerrainFilteredMaterial {
  UTerrainMaterial* Material = nullptr;
  float Alpha = 1.f;
};

class UTerrainLayerSetup : public UObject {
public:
  DECL_UOBJ(UTerrainLayerSetup, UObject);

  UPROP_NOINIT(std::vector<FTerrainFilteredMaterial>, Materials);

  bool RegisterProperty(FPropertyTag* property) override;

  void PostLoad() override;
};

struct FTerrainLayer {
  FString Name;
  UTerrainLayerSetup* Setup = nullptr;
  int32 AlphaMapIndex = 0;
  bool Hidden = false;
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
  UPROP_NOINIT(std::vector<FTerrainLayer>, Layers);

  bool RegisterProperty(FPropertyTag* property) override;
  void Serialize(FStream& s) override;
  void PostLoad() override;
  
  void GetHeightMap(uint16*& result, int32& width, int32& height, bool resample = false);
  void GetVisibilityMap(uint8*& result, int32& width, int32& height, bool resample = false);
  float GetHeightMapRatioX();
  float GetHeightMapRatioY();

  bool GetWeightMapChannel(int32 idx, void*& data, int32& width, int32& height);
  std::vector<UTerrainWeightMapTexture*> GetWeightMaps();

  inline bool HasVisibilityData()
  {
    return HasTransparency;
  }

  inline int32 GetAlphaMapsCount() const
  {
    return (int32)AlphaMaps.size();
  }

protected:
  std::vector<FTerrainHeight> Heights;
  std::vector<FTerrainInfoData> InfoData;
  int32 Unk1 = 0;
  std::vector<FAlphaMap> AlphaMaps;
  FObjectArray<UTerrainWeightMapTexture*> WeightMapTextures;
  void* MaterialData = nullptr;
  FILE_OFFSET MaterialDataSize = 0;
  bool HasTransparency = false;
  std::vector<FTerrainMaterialResource*> CachedTerrainMaterials;
  uint8* DisplacementMap = nullptr;
  FILE_OFFSET DisplacementMapSize = 0;
  float MaxCollisionDisplacement = 0.;
};