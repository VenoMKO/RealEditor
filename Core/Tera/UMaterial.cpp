#include "UMaterial.h"
#include "UTexture.h"
#include "FPackage.h"
#include "Cast.h"


FStream& operator<<(FStream& s, FTextureLookup& l)
{
  s << l.TexCoordIndex;
  s << l.TextureIndex;
  s << l.UScale;
  s << l.VScale;
  return s;
}

FStream& operator<<(FStream& s, FStaticSwitchParameter& p)
{
  return s << p.ParameterName << p.Value << p.bOverride << p.ExpressionGUID;
}

FStream& operator<<(FStream& s, FStaticComponentMaskParameter& p)
{
  return s << p.ParameterName << p.R << p.G << p.B << p.A << p.bOverride << p.ExpressionGUID;
}

FStream& operator<<(FStream& s, FNormalParameter& p)
{
  return s << p.ParameterName << p.CompressionSettings << p.bOverride << p.ExpressionGUID;
}

FStream& operator<<(FStream& s, FStaticTerrainLayerWeightParameter& p)
{
  return s << p.ParameterName << p.WeightmapIndex << p.bOverride << p.ExpressionGUID;
}

FStream& operator<<(FStream& s, FStaticParameterSet& ps)
{
  s << ps.BaseMaterialId;
  s << ps.StaticSwitchParameters;
  s << ps.StaticComponentMaskParameters;
  s << ps.NormalParameters;
  s << ps.TerrainLayerWeightParameters;
  return s;
}

void FMaterial::Serialize(FStream& s)
{
  s << CompoilerErrors;
  s << TextureDependencyLengthMap;
  s << MaxTextureDependencyLength;
  s << Id;
  s << NumUserTexCoords;
  s << UniformExpressionTextures;
  s << bUsesSceneColor;
  s << bUsesSceneDepth;
  s << bUsesDynamicParameter;
  s << bUsesLightmapUVs;
  s << bUsesMaterialVertexPositionOffset;
  s << UsingTransforms;
  s << TextureLookups;
  s << FallbackComponents;
  s << Unk1;
  s << Unk2;
  s << Unk3;
}

bool UMaterialInterface::RegisterProperty(FPropertyTag* property)
{
  if (PROP_IS(property, TextureParameterValues))
  {
    TextureParameterValues = property->Value->GetArray();
    TextureParameterValuesProperty = property;
    return true;
  }
  if (PROP_IS(property, BlendMode))
  {
    BlendMode = (EBlendMode)property->Value->GetByte();
    BlendModeProperty = property;
    return true;
  }
  if (PROP_IS(property, Parent))
  {
    Parent = property->Value->GetObjectIndex();
    ParentProperty = property;
    return true;
  }
  return false;
}

UTexture2D* UMaterialInterface::GetDiffuseTexture() const
{
  UTexture2D* result = GetTextureParameterValue("PCC_HairDiffuseMap");
  if (!result)
  {
    result = GetTextureParameterValue("DiffuseMap");
  }
  return result;
}

EBlendMode UMaterialInterface::GetBlendMode() const
{
  if (BlendModeProperty)
  {
    return BlendMode;
  }
  UMaterialInterface* parent = Cast<UMaterialInterface>(GetParent());
  while (parent)
  {
    if (parent->BlendModeProperty)
    {
      return parent->GetBlendMode();
    }
    parent = Cast<UMaterialInterface>(parent->GetParent());
  }
  return BlendMode;
}

UObject* UMaterialInterface::GetParent() const
{
  return ParentProperty ? GetPackage()->GetObject(Parent) : nullptr;
}

UTexture2D* UMaterialInterface::GetTextureParameterValue(const FString& name) const
{
  bool thisValue = false;
  for (FPropertyValue* container : TextureParameterValues)
  {
    std::vector<FPropertyValue*>& tmpArray = container->GetArray();
    for (FPropertyValue* subcontainer : tmpArray)
    {
      FPropertyTag* tmpTag = subcontainer->GetPropertyTagPtr();
      if (tmpTag->Name == "ParameterName" && tmpTag->Value->GetName() == name)
      {
        thisValue = true;
        continue;
      }
      else if (thisValue && tmpTag->Name == "ParameterValue")
      {
        PACKAGE_INDEX objIndex = tmpTag->Value->GetObjectIndex();
        thisValue = false;
        if (objIndex)
        {
          return Cast<UTexture2D>(GetPackage()->GetObject(objIndex));
        }
      }
    }
  }
  return nullptr;
}

void UMaterial::Serialize(FStream& s)
{
  Super::Serialize(s);
  MaterialResource.Serialize(s);
  SerializeTrailingData(s);
}

void UMaterialInstance::Serialize(FStream& s)
{
  Super::Serialize(s);
  if (!bHasStaticPermutationResource)
  {
    return;
  }

  StaticPermutationResource.Serialize(s);
  s << StaticParameters;

  SerializeTrailingData(s);
}

bool UMaterialInstance::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, bHasStaticPermutationResource))
  {
    bHasStaticPermutationResource = property->BoolVal;
    bHasStaticPermutationResourceProperty = property;
    return true;
  }
  return false;
}
