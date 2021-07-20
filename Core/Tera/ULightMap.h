#pragma once
#include "Core.h"
#include "FStructs.h"

#define NUM_STORED_LIGHTMAP_COEF 3
#define NUM_DIRECTIONAL_LIGHTMAP_COEF 2
#define NUM_LEGACY_DIRECTIONAL_LIGHTMAP_COEF 3
#define NUM_SIMPLE_LIGHTMAP_COEF 1

struct FQuantizedDirectionalLightSample {
  FColor Coefficients[NUM_DIRECTIONAL_LIGHTMAP_COEF];
};

struct FLegacyQuantizedDirectionalLightSample {
  FColor Coefficients[NUM_LEGACY_DIRECTIONAL_LIGHTMAP_COEF];
};

struct FQuantizedSimpleLightSample {
  FColor Coefficients[NUM_SIMPLE_LIGHTMAP_COEF];
};

template<class QuantizedLightSampleType>
struct FQuantizedLightSampleBulkData : public FUntypedBulkData {
  using SampleType = QuantizedLightSampleType;
  bool RequiresSingleElementSerialization(FStream& Ar) override;
  int32 GetElementSize() const override;
  void SerializeElement(FStream& s, void* data, int32 elementIndex) override;
};

class FLightMap {
public:
  enum
  {
    LMT_None = 0,
    LMT_1D = 1,
    LMT_2D = 2,
  };
  FLightMap() = default;
  FLightMap(int32 type)
    : Type(type)
  {}

  virtual ~FLightMap()
  {}

  virtual void Serialize(FStream& s);

  uint32 Type = LMT_None;
  std::vector<FGuid> LightGuids;
};

class FLightMap1D : public FLightMap {
public:
  FLightMap1D()
    : FLightMap(LMT_1D)
  {}

  ~FLightMap1D();

  void Serialize(FStream& s) override;

protected:
  DECL_UREF(UObject, Owner);
  FQuantizedLightSampleBulkData<FQuantizedDirectionalLightSample>* DirectionalSamples = nullptr;
  FQuantizedLightSampleBulkData<FLegacyQuantizedDirectionalLightSample>* LegacyDirectionalSamples = nullptr;
  FVector ScaleVectors[NUM_STORED_LIGHTMAP_COEF];
  FVector LegacyScaleVectors[4];
  FQuantizedLightSampleBulkData<FQuantizedSimpleLightSample>* SimpleSamples = nullptr;
};

class FLightMap2D : public FLightMap {
public:
  FLightMap2D()
    : FLightMap(LMT_2D)
  {}

  ~FLightMap2D()
  {}

  void Serialize(FStream& s) override;

protected:
  DECL_UREF(UObject, Texture1);
  DECL_UREF(UObject, Texture2);
  DECL_UREF(UObject, Texture3);
  FVector ScaleVectors[NUM_STORED_LIGHTMAP_COEF];
  FVector2D CoordinateScale;
  FVector2D CoordinateBias;
};
