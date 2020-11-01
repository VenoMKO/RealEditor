#include "UTerrain.h"
#include "FPackage.h"
#include "Cast.h"

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
}