#include "FObjectResource.h"
#include "UMaterial.h"
#include "UMaterialExpression.h"
#include "UTexture.h"
#include "FPackage.h"
#include "Cast.h"

class MaterialExpressionNormalAlphaVisitor : public UMaterialExpressionViewVisitor {
public:
  ~MaterialExpressionNormalAlphaVisitor()
  {}

  void SetTitle(const FString& title) override
  {}
  void SetValue(const FString& value) override
  {}
  void SetDescription(const FString& desc) override
  {}
  void SetInput(const std::vector<FExpressionInput>& input) override
  {
    for (const FExpressionInput& in : input)
    {
      if (in.MaskA && in.Expression == SearchExpression)
      {
        Found = true;
        break;
      }
    }
  }
  void SetEditorPosition(int32 x, int32 y) override
  {}
  void SetEditorSize(int32 x, int32 y) override
  {}

  UMaterialExpression* SearchExpression = nullptr;
  bool Found = false;
};

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

void FMaterial::Serialize(FStream& s, UObject* owner)
{
  s << Unk1;
  s << CompoilerErrors;
  if (!owner || !owner->IsTransacting())
  {
    // Transacting the map causes a random access violation during game play.
    // Probably caused by the fact that the map keys aren't serialized properly.
    // Meaning that after transaction object indices are totally messed up.
    s << TextureDependencyLengthMap;
  }
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
  s << Unk2;
  s << Unk3;
  s << Unk4;
}

bool UMaterialInterface::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, ScalarParameterValues))
  {
    ScalarParameterValues = property->Value->GetArray();
    ScalarParameterValuesProperty = property;
    return true;
  }
  if (PROP_IS(property, TextureParameterValues))
  {
    TextureParameterValues = property->Value->GetArray();
    TextureParameterValuesProperty = property;
    return true;
  }
  if (PROP_IS(property, VectorParameterValues))
  {
    VectorParameterValues = property->Value->GetArray();
    VectorParameterValuesProperty = property;
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

std::map<FString, float> UMaterialInterface::GetScalarParameters() const
{
  std::map<FString, float> result;

  if (UMaterial* mat = Cast<UMaterial>(this))
  {
    std::vector<UMaterialExpression*> expressions = mat->GetExpressions();
    for (UMaterialExpression* exp : expressions)
    {
      if (UMaterialExpressionScalarParameter* sexp = Cast<UMaterialExpressionScalarParameter>(exp))
      {
        result[sexp->ParameterName.String()] = sexp->DefaultValue;
      }
    }
  }

  for (FPropertyValue* container : ScalarParameterValues)
  {
    std::vector<FPropertyValue*>& tmpArray = container->GetArray();
    FString parameterName;
    float parameterValue = 0.;
    for (FPropertyValue* subcontainer : tmpArray)
    {
      FPropertyTag* tmpTag = subcontainer->GetPropertyTagPtr();
      if (tmpTag->Name == "ParameterName")
      {
        parameterName = tmpTag->GetName().String();
        continue;
      }
      if (tmpTag->Name == "ParameterValue")
      {
        parameterValue = tmpTag->GetFloat();
      }
    }
    if (parameterName.Size())
    {
      result[parameterName] = parameterValue;
    }
  }
  return result;
}

std::map<FString, FTextureParameter> UMaterialInterface::GetTextureParameters() const
{
  std::map<FString, FTextureParameter> result;
  if (UMaterial* mat = Cast<UMaterial>(this))
  {
    std::vector<UMaterialExpression*> expressions = mat->GetExpressions();
    for (UMaterialExpression* exp : expressions)
    {
      if (UMaterialExpressionTextureSampleParameter* sexp = Cast<UMaterialExpressionTextureSampleParameter>(exp))
      {
        result[sexp->ParameterName.String()].Texture = Cast<UTexture>(sexp->Texture);

        MaterialExpressionNormalAlphaVisitor visitor;
        visitor.SearchExpression = sexp;

        for (UMaterialExpression* e : expressions)
        {
          if (e == exp)
          {
            continue;
          }
          e->AcceptVisitor(&visitor);
          if (visitor.Found)
          {
            break;
          }
        }

        if (visitor.Found)
        {
          result[sexp->ParameterName.String()].AlphaChannelUsed = true;
        }
      }
    }
  }
  else if (UMaterialInterface* parent = Cast<UMaterialInterface>(GetParent()))
  {
    while (parent->GetParent())
    {
      parent = Cast<UMaterialInterface>(parent->GetParent());
    }
    if (UMaterial* mat = Cast<UMaterial>(parent))
    {
      std::vector<UMaterialExpression*> expressions = mat->GetExpressions();
      for (UMaterialExpression* exp : expressions)
      {
        if (UMaterialExpressionTextureSampleParameter* sexp = Cast<UMaterialExpressionTextureSampleParameter>(exp))
        {
          MaterialExpressionNormalAlphaVisitor visitor;
          visitor.SearchExpression = sexp;

          for (UMaterialExpression* e : expressions)
          {
            if (e == exp)
            {
              continue;
            }
            e->AcceptVisitor(&visitor);
            if (visitor.Found)
            {
              break;
            }
          }

          result[sexp->ParameterName.String()].AlphaChannelUsed = visitor.Found;
        }
      }
    }
  }

  for (FPropertyValue* container : TextureParameterValues)
  {
    std::vector<FPropertyValue*>& tmpArray = container->GetArray();
    FString parameterName;
    UTexture* parameterValue = nullptr;
    for (FPropertyValue* subcontainer : tmpArray)
    {
      FPropertyTag* tmpTag = subcontainer->GetPropertyTagPtr();
      if (tmpTag->Name == "ParameterName")
      {
        FString tmp = tmpTag->GetName().String();
        if (!result.count(tmp))
        {
          break;
        }
        parameterName = tmp;
        continue;
      }
      if (tmpTag->Name == "ParameterValue")
      {
        parameterValue = Cast<UTexture>(GetPackage()->GetObject(tmpTag->Value->GetObjectIndex()));
      }
    }
    if (parameterName.Size())
    {
      result[parameterName].Texture = parameterValue;
    }
  }
  return result;
}

std::map<FString, UTexture*> UMaterialInterface::GetTextureCubeParameters() const
{
  std::map<FString, UTexture*> result;
  
  std::vector<FString> cubeParameterNames;
  if (UMaterial* mat = Cast<UMaterial>(this))
  {
    std::vector<UMaterialExpression*> expressions = mat->GetExpressions();
    for (UMaterialExpression* exp : expressions)
    {
      if (UMaterialExpressionTextureSampleParameterCube* sexp = Cast<UMaterialExpressionTextureSampleParameterCube>(exp))
      {
        result[sexp->ParameterName.String()] = Cast<UTexture>(sexp->Texture);
      }
    }
  }
  else if (UMaterialInterface* parent = Cast<UMaterialInterface>(GetParent()))
  {
    while (parent->GetParent())
    {
      parent = Cast<UMaterialInterface>(parent->GetParent());
    }
    if (UMaterial* mat = Cast<UMaterial>(parent))
    {
      std::vector<UMaterialExpression*> expressions = mat->GetExpressions();
      for (UMaterialExpression* exp : expressions)
      {
        if (UMaterialExpressionTextureSampleParameterCube* sexp = Cast<UMaterialExpressionTextureSampleParameterCube>(exp))
        {
          cubeParameterNames.emplace_back(sexp->ParameterName.String());
        }
      }
    }
  }

  for (FPropertyValue* container : TextureParameterValues)
  {
    std::vector<FPropertyValue*>& tmpArray = container->GetArray();
    FString parameterName;
    UTexture* parameterValue = nullptr;
    for (FPropertyValue* subcontainer : tmpArray)
    {
      FPropertyTag* tmpTag = subcontainer->GetPropertyTagPtr();
      if (tmpTag->Name == "ParameterName")
      {
        FString tmp = tmpTag->GetName().String();
        if (std::find(cubeParameterNames.begin(), cubeParameterNames.end(), tmp) == cubeParameterNames.end())
        {
          break;
        }
        parameterName = tmp;
        continue;
      }
      if (tmpTag->Name == "ParameterValue")
      {
        if (UTextureCube* cube = Cast<UTextureCube>(GetPackage()->GetObject(tmpTag->Value->GetObjectIndex())))
        {
          parameterValue = cube;
        }
        else
        {
          continue;
        }
      }
    }
    if (parameterName.Size())
    {
      result[parameterName] = parameterValue;
    }
  }
  return result;
}

std::map<FString, FLinearColor> UMaterialInterface::GetVectorParameters() const
{
  std::map<FString, FLinearColor> result;

  if (UMaterial* mat = Cast<UMaterial>(this))
  {
    std::vector<UMaterialExpression*> expressions = mat->GetExpressions();
    for (UMaterialExpression* exp : expressions)
    {
      if (UMaterialExpressionVectorParameter* sexp = Cast<UMaterialExpressionVectorParameter>(exp))
      {
        result[sexp->ParameterName.String()] = sexp->DefaultValue;
      }
    }
  }

  for (FPropertyValue* container : VectorParameterValues)
  {
    std::vector<FPropertyValue*>& tmpArray = container->GetArray();
    FString parameterName;
    FLinearColor parameterValue;
    for (FPropertyValue* subcontainer : tmpArray)
    {
      FPropertyTag* tmpTag = subcontainer->GetPropertyTagPtr();
      if (tmpTag->Name == "ParameterName")
      {
        parameterName = tmpTag->GetName().String();
        continue;
      }
      if (tmpTag->Name == "ParameterValue")
      {
        tmpTag->GetLinearColor(parameterValue);
      }
    }
    if (parameterName.Size())
    {
      result[parameterName] = parameterValue;
    }
  }
  return result;
}

std::vector<UTexture*> UMaterialInterface::GetTextureSamples() const
{
  std::vector<UTexture*> result;
  if (UMaterial* mat = Cast<UMaterial>(this))
  {
    std::vector<UMaterialExpression*> expressions = mat->GetExpressions();
    for (UMaterialExpression* exp : expressions)
    {
      if (UMaterialExpressionTextureSample* sexp = ExactCast<UMaterialExpressionTextureSample>(exp))
      {
        if (UTexture* tex = Cast<UTexture>(sexp->Texture))
        {
          result.push_back(tex);
        }
      }
    }
  }
  else if (UMaterialInterface* parent = Cast<UMaterialInterface>(GetParent()))
  {
    result = parent->GetTextureSamples();
  }
  return result;
}

std::map<FString, bool> UMaterialInterface::GetStaticBoolParameters() const
{
  std::map<FString, bool> result;
  if (UMaterial* mat = Cast<UMaterial>(this))
  {
    std::vector<UMaterialExpression*> expressions = mat->GetExpressions();
    for (UMaterialExpression* exp : expressions)
    {
      if (UMaterialExpressionStaticSwitchParameter* sexp = Cast<UMaterialExpressionStaticSwitchParameter>(exp))
      {
        result[sexp->ParameterName.String()] = sexp->DefaultValue;
      }
    }
  }
  else if (UMaterialInstance* mi = Cast<UMaterialInstance>(this))
  {
    if (mi->bHasStaticPermutationResource)
    {
      for (const FStaticSwitchParameter& param : mi->StaticParameters.StaticSwitchParameters)
      {
        if (param.bOverride)
        {
          result[param.ParameterName.String()] = param.Value;
        }
      }
    }
  }
  return result;
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
  MaterialResource.Serialize(s, this);
}

bool UMaterial::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(TwoSided);
  if (property->StructName.String().Find("MaterialInput") != std::string::npos)
  {
    MaterialInputs.push_back(property);
  }
  return false;
}

std::vector<class UMaterialExpression*> UMaterial::GetExpressions() const
{
  std::vector<UMaterialExpression*> result;
  auto inner = GetInner();
  for (UObject* i : inner)
  {
    if (UMaterialExpression* exp = Cast<UMaterialExpression>(i))
    {
      exp->Load();
      result.push_back(exp);
    }
  }
  return result;
}

void UMaterialInstance::Serialize(FStream& s)
{
  Super::Serialize(s);
  if (!bHasStaticPermutationResource)
  {
    return;
  }
  StaticPermutationResource.Serialize(s, this);
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