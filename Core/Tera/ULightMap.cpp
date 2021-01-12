#include "ULightMap.h"
#include "FStream.h"

template<class QuantizedLightSampleType>
bool FQuantizedLightSampleBulkData<QuantizedLightSampleType>::RequiresSingleElementSerialization(FStream&)
{
  return false;
}

template<class QuantizedLightSampleType>
int32 FQuantizedLightSampleBulkData<QuantizedLightSampleType>::GetElementSize() const
{
  return (int32)sizeof(QuantizedLightSampleType);
}

template<class QuantizedLightSampleType>
void FQuantizedLightSampleBulkData<QuantizedLightSampleType>::SerializeElement(FStream& s, void* data, int32 elementIndex)
{
  QuantizedLightSampleType* sample = (QuantizedLightSampleType*)data + elementIndex;
  const uint32 numCoefficients = sizeof(QuantizedLightSampleType) / sizeof(FColor);
  for (int32 idx = 0; idx < numCoefficients; ++idx)
  {
    uint32 color = sample->Coefficients[idx].DWColor();
    s << color;
    sample->Coefficients[idx] = FColor(color);
  }
};

void FLightMap::Serialize(FStream& s)
{
  if (!s.IsReading())
  {
    s << Type;
  }
  s << LightGuids;
}

FLightMap1D::~FLightMap1D()
{
  if (DirectionalSamples)
  {
    delete DirectionalSamples;
  }
  if (SimpleSamples)
  {
    delete SimpleSamples;
  }
}

void FLightMap1D::Serialize(FStream& s)
{
  FLightMap::Serialize(s);
  SERIALIZE_UREF(s, Owner);
  if (s.IsReading())
  {
    DirectionalSamples = new FQuantizedLightSampleBulkData<FQuantizedDirectionalLightSample>();
    DirectionalSamples->Package = s.GetPackage();
    SimpleSamples = new FQuantizedLightSampleBulkData<FQuantizedSimpleLightSample>();
    SimpleSamples->Package = s.GetPackage();
  }
  DirectionalSamples->Serialize(s, Owner);

  for (auto& ScaleVector : ScaleVectors)
  {
    s << ScaleVector;
  }

  SimpleSamples->Serialize(s, Owner);
}

void FLightMap2D::Serialize(FStream& s)
{
  FLightMap::Serialize(s);
  SERIALIZE_UREF(s, Texture1);
  s << ScaleVectors[0];
  SERIALIZE_UREF(s, Texture2);
  s << ScaleVectors[1];
  SERIALIZE_UREF(s, Texture3);
  s << ScaleVectors[2];

  s << CoordinateScale;
  s << CoordinateBias;
}
