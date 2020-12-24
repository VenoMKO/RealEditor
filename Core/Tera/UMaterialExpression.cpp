#include "UMaterialExpression.h"
#include "FObjectResource.h"
#include "UClass.h"
#include "Cast.h"

#define REGISTER_INPUT(TName)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  TName = FExpressionInput(property);\
  TName.Title = #TName;\
  return true;\
}\
//

#define SUPER_ACCEPT()\
if (!visitor)\
{\
  return;\
}\
Super::AcceptVisitor(visitor)

#define SET_INPUT(...)\
visitor->SetInput({ __VA_ARGS__ });

FExpressionInput::FExpressionInput(FPropertyTag * property)
{
  if (property && property->Value)
  {
    std::vector<FPropertyValue*> arr = property->Value->GetArray();
    for (FPropertyValue* item : arr)
    {
      FPropertyTag* tag = item->GetPropertyTagPtr();
      FString tagName = tag->Name.String();
      if (tagName == "Expression")
      {
        Expression = Cast<UMaterialExpression>(tag->GetObjectValuePtr());
      }
      else if (tagName == "Mask")
      {
        Mask = tag->Value ? tag->Value->GetBool() : tag->GetBool();
      }
      else if (tagName == "MaskR")
      {
        MaskR = tag->Value ? tag->Value->GetBool() : tag->GetBool();
      }
      else if (tagName == "MaskG")
      {
        MaskG = tag->Value ? tag->Value->GetBool() : tag->GetBool();
      }
      else if (tagName == "MaskB")
      {
        MaskB = tag->Value ? tag->Value->GetBool() : tag->GetBool();
      }
      else if (tagName == "MaskA")
      {
        MaskA = tag->Value ? tag->Value->GetBool() : tag->GetBool();
      }
    }
  }
}

FString FExpressionInput::GetDescription() const
{
  return Title.Empty() ? FString("In") : Title;
}

UMaterialExpression* UMaterialExpression::StaticFactory(FObjectExport* exp)
{
  UMaterialExpression* result = nullptr;
  const FString c = exp->GetClassName();

  if (c == UMaterialExpressionAbs::StaticClassName())
  {
    result = new UMaterialExpressionAbs(exp);
  }
  else if (c == UMaterialExpressionAdd::StaticClassName())
  {
    result = new UMaterialExpressionAdd(exp);
  }
  else if (c == UMaterialExpressionAppendVector::StaticClassName())
  {
    result = new UMaterialExpressionAppendVector(exp);
  }
  else if (c == UMaterialExpressionBumpOffset::StaticClassName())
  {
    result = new UMaterialExpressionBumpOffset(exp);
  }
  else if (c == UMaterialExpressionCameraVector::StaticClassName())
  {
    result = new UMaterialExpressionCameraVector(exp);
  }
  else if (c == UMaterialExpressionCameraWorldPosition::StaticClassName())
  {
    result = new UMaterialExpressionCameraWorldPosition(exp);
  }
  else if (c == UMaterialExpressionCeil::StaticClassName())
  {
    result = new UMaterialExpressionCeil(exp);
  }
  else if (c == UMaterialExpressionClamp::StaticClassName())
  {
    result = new UMaterialExpressionClamp(exp);
  }
  else if (c == UMaterialExpressionComment::StaticClassName())
  {
    result = new UMaterialExpressionComment(exp);
  }
  else if (c == UMaterialExpressionComponentMask::StaticClassName())
  {
    result = new UMaterialExpressionComponentMask(exp);
  }
  else if (c == UMaterialExpressionCompound::StaticClassName())
  {
    result = new UMaterialExpressionCompound(exp);
  }
  else if (c == UMaterialExpressionConstant::StaticClassName())
  {
    result = new UMaterialExpressionConstant(exp);
  }
  else if (c == UMaterialExpressionConstant2Vector::StaticClassName())
  {
    result = new UMaterialExpressionConstant2Vector(exp);
  }
  else if (c == UMaterialExpressionConstant3Vector::StaticClassName())
  {
    result = new UMaterialExpressionConstant3Vector(exp);
  }
  else if (c == UMaterialExpressionConstant4Vector::StaticClassName())
  {
    result = new UMaterialExpressionConstant4Vector(exp);
  }
  else if (c == UMaterialExpressionConstantBiasScale::StaticClassName())
  {
    result = new UMaterialExpressionConstantBiasScale(exp);
  }
  else if (c == UMaterialExpressionConstantClamp::StaticClassName())
  {
    result = new UMaterialExpressionConstantClamp(exp);
  }
  else if (c == UMaterialExpressionCosine::StaticClassName())
  {
    result = new UMaterialExpressionCosine(exp);
  }
  else if (c == UMaterialExpressionCrossProduct::StaticClassName())
  {
    result = new UMaterialExpressionCrossProduct(exp);
  }
  else if (c == UMaterialExpressionCustomTexture::StaticClassName())
  {
    result = new UMaterialExpressionCustomTexture(exp);
  }
  else if (c == UMaterialExpressionDepthBiasedAlpha::StaticClassName())
  {
    result = new UMaterialExpressionDepthBiasedAlpha(exp);
  }
  else if (c == UMaterialExpressionDepthBiasedBlend::StaticClassName())
  {
    result = new UMaterialExpressionDepthBiasedBlend(exp);
  }
  else if (c == UMaterialExpressionDepthOfFieldFunction::StaticClassName())
  {
    result = new UMaterialExpressionDepthOfFieldFunction(exp);
  }
  else if (c == UMaterialExpressionDeriveNormalZ::StaticClassName())
  {
    result = new UMaterialExpressionDeriveNormalZ(exp);
  }
  else if (c == UMaterialExpressionDesaturation::StaticClassName())
  {
    result = new UMaterialExpressionDesaturation(exp);
  }
  else if (c == UMaterialExpressionDestColor::StaticClassName())
  {
    result = new UMaterialExpressionDestColor(exp);
  }
  else if (c == UMaterialExpressionDestDepth::StaticClassName())
  {
    result = new UMaterialExpressionDestDepth(exp);
  }
  else if (c == UMaterialExpressionDistance::StaticClassName())
  {
    result = new UMaterialExpressionDistance(exp);
  }
  else if (c == UMaterialExpressionDivide::StaticClassName())
  {
    result = new UMaterialExpressionDivide(exp);
  }
  else if (c == UMaterialExpressionDotProduct::StaticClassName())
  {
    result = new UMaterialExpressionDotProduct(exp);
  }
  else if (c == UMaterialExpressionDynamicParameter::StaticClassName())
  {
    result = new UMaterialExpressionDynamicParameter(exp);
  }
  else if (c == UMaterialExpressionMeshEmitterDynamicParameter::StaticClassName())
  {
    result = new UMaterialExpressionMeshEmitterDynamicParameter(exp);
  }
  else if (c == UMaterialExpressionFloor::StaticClassName())
  {
    result = new UMaterialExpressionFloor(exp);
  }
  else if (c == UMaterialExpressionFluidNormal::StaticClassName())
  {
    result = new UMaterialExpressionFluidNormal(exp);
  }
  else if (c == UMaterialExpressionFmod::StaticClassName())
  {
    result = new UMaterialExpressionFmod(exp);
  }
  else if (c == UMaterialExpressionFoliageImpulseDirection::StaticClassName())
  {
    result = new UMaterialExpressionFoliageImpulseDirection(exp);
  }
  else if (c == UMaterialExpressionFoliageNormalizedRotationAxisAndAngle::StaticClassName())
  {
    result = new UMaterialExpressionFoliageNormalizedRotationAxisAndAngle(exp);
  }
  else if (c == UMaterialExpressionFontSample::StaticClassName())
  {
    result = new UMaterialExpressionFontSample(exp);
  }
  else if (c == UMaterialExpressionFontSampleParameter::StaticClassName())
  {
    result = new UMaterialExpressionFontSampleParameter(exp);
  }
  else if (c == UMaterialExpressionFrac::StaticClassName())
  {
    result = new UMaterialExpressionFrac(exp);
  }
  else if (c == UMaterialExpressionFresnel::StaticClassName())
  {
    result = new UMaterialExpressionFresnel(exp);
  }
  else if (c == UMaterialExpressionIf::StaticClassName())
  {
    result = new UMaterialExpressionIf(exp);
  }
  else if (c == UMaterialExpressionLensFlareIntensity::StaticClassName())
  {
    result = new UMaterialExpressionLensFlareIntensity(exp);
  }
  else if (c == UMaterialExpressionLensFlareOcclusion::StaticClassName())
  {
    result = new UMaterialExpressionLensFlareOcclusion(exp);
  }
  else if (c == UMaterialExpressionLensFlareRadialDistance::StaticClassName())
  {
    result = new UMaterialExpressionLensFlareRadialDistance(exp);
  }
  else if (c == UMaterialExpressionLensFlareRayDistance::StaticClassName())
  {
    result = new UMaterialExpressionLensFlareRayDistance(exp);
  }
  else if (c == UMaterialExpressionLensFlareSourceDistance::StaticClassName())
  {
    result = new UMaterialExpressionLensFlareSourceDistance(exp);
  }
  else if (c == UMaterialExpressionLightmapUVs::StaticClassName())
  {
    result = new UMaterialExpressionLightmapUVs(exp);
  }
  else if (c == UMaterialExpressionLightmassReplace::StaticClassName())
  {
    result = new UMaterialExpressionLightmassReplace(exp);
  }
  else if (c == UMaterialExpressionLightVector::StaticClassName())
  {
    result = new UMaterialExpressionLightVector(exp);
  }
  else if (c == UMaterialExpressionLinearInterpolate::StaticClassName())
  {
    result = new UMaterialExpressionLinearInterpolate(exp);
  }
  else if (c == UMaterialExpressionMeshEmitterVertexColor::StaticClassName())
  {
    result = new UMaterialExpressionMeshEmitterVertexColor(exp);
  }
  else if (c == UMaterialExpressionMultiply::StaticClassName())
  {
    result = new UMaterialExpressionMultiply(exp);
  }
  else if (c == UMaterialExpressionNormalize::StaticClassName())
  {
    result = new UMaterialExpressionNormalize(exp);
  }
  else if (c == UMaterialExpressionObjectOrientation::StaticClassName())
  {
    result = new UMaterialExpressionObjectOrientation(exp);
  }
  else if (c == UMaterialExpressionObjectRadius::StaticClassName())
  {
    result = new UMaterialExpressionObjectRadius(exp);
  }
  else if (c == UMaterialExpressionObjectWorldPosition::StaticClassName())
  {
    result = new UMaterialExpressionObjectWorldPosition(exp);
  }
  else if (c == UMaterialExpressionOcclusionPercentage::StaticClassName())
  {
    result = new UMaterialExpressionOcclusionPercentage(exp);
  }
  else if (c == UMaterialExpressionOneMinus::StaticClassName())
  {
    result = new UMaterialExpressionOneMinus(exp);
  }
  else if (c == UMaterialExpressionPanner::StaticClassName())
  {
    result = new UMaterialExpressionPanner(exp);
  }
  else if (c == UMaterialExpressionParameter::StaticClassName())
  {
    result = new UMaterialExpressionParameter(exp);
  }
  else if (c == UMaterialExpressionScalarParameter::StaticClassName())
  {
    result = new UMaterialExpressionScalarParameter(exp);
  }
  else if (c == UMaterialExpressionStaticComponentMaskParameter::StaticClassName())
  {
    result = new UMaterialExpressionStaticComponentMaskParameter(exp);
  }
  else if (c == UMaterialExpressionStaticSwitchParameter::StaticClassName())
  {
    result = new UMaterialExpressionStaticSwitchParameter(exp);
  }
  else if (c == UMaterialExpressionVectorParameter::StaticClassName())
  {
    result = new UMaterialExpressionVectorParameter(exp);
  }
  else if (c == UMaterialExpressionParticleMacroUV::StaticClassName())
  {
    result = new UMaterialExpressionParticleMacroUV(exp);
  }
  else if (c == UMaterialExpressionPerInstanceRandom::StaticClassName())
  {
    result = new UMaterialExpressionPerInstanceRandom(exp);
  }
  else if (c == UMaterialExpressionPixelDepth::StaticClassName())
  {
    result = new UMaterialExpressionPixelDepth(exp);
  }
  else if (c == UMaterialExpressionPower::StaticClassName())
  {
    result = new UMaterialExpressionPower(exp);
  }
  else if (c == UMaterialExpressionReflectionVector::StaticClassName())
  {
    result = new UMaterialExpressionReflectionVector(exp);
  }
  else if (c == UMaterialExpressionRotateAboutAxis::StaticClassName())
  {
    result = new UMaterialExpressionRotateAboutAxis(exp);
  }
  else if (c == UMaterialExpressionRotator::StaticClassName())
  {
    result = new UMaterialExpressionRotator(exp);
  }
  else if (c == UMaterialExpressionSceneDepth::StaticClassName())
  {
    result = new UMaterialExpressionSceneDepth(exp);
  }
  else if (c == UMaterialExpressionSceneTexture::StaticClassName())
  {
    result = new UMaterialExpressionSceneTexture(exp);
  }
  else if (c == UMaterialExpressionScreenPosition::StaticClassName())
  {
    result = new UMaterialExpressionScreenPosition(exp);
  }
  else if (c == UMaterialExpressionSine::StaticClassName())
  {
    result = new UMaterialExpressionSine(exp);
  }
  else if (c == UMaterialExpressionSphereMask::StaticClassName())
  {
    result = new UMaterialExpressionSphereMask(exp);
  }
  else if (c == UMaterialExpressionSquareRoot::StaticClassName())
  {
    result = new UMaterialExpressionSquareRoot(exp);
  }
  else if (c == UMaterialExpressionSubtract::StaticClassName())
  {
    result = new UMaterialExpressionSubtract(exp);
  }
  else if (c == UMaterialExpressionTerrainLayerCoords::StaticClassName())
  {
    result = new UMaterialExpressionTerrainLayerCoords(exp);
  }
  else if (c == UMaterialExpressionTerrainLayerWeight::StaticClassName())
  {
    result = new UMaterialExpressionTerrainLayerWeight(exp);
  }
  else if (c == UMaterialExpressionTextureCoordinate::StaticClassName())
  {
    result = new UMaterialExpressionTextureCoordinate(exp);
  }
  else if (c == UMaterialExpressionTextureSample::StaticClassName())
  {
    result = new UMaterialExpressionTextureSample(exp);
  }
  else if (c == UMaterialExpressionDepthBiasBlend::StaticClassName())
  {
    result = new UMaterialExpressionDepthBiasBlend(exp);
  }
  else if (c == UMaterialExpressionFlipBookSample::StaticClassName())
  {
    result = new UMaterialExpressionFlipBookSample(exp);
  }
  else if (c == UMaterialExpressionMeshSubUV::StaticClassName())
  {
    result = new UMaterialExpressionMeshSubUV(exp);
  }
  else if (c == UMaterialExpressionMeshSubUVBlend::StaticClassName())
  {
    result = new UMaterialExpressionMeshSubUVBlend(exp);
  }
  else if (c == UMaterialExpressionParticleSubUV::StaticClassName())
  {
    result = new UMaterialExpressionParticleSubUV(exp);
  }
  else if (c == UMaterialExpressionTextureSampleParameter::StaticClassName())
  {
    result = new UMaterialExpressionTextureSampleParameter(exp);
  }
  else if (c == UMaterialExpressionTextureSampleParameter2D::StaticClassName())
  {
    result = new UMaterialExpressionTextureSampleParameter2D(exp);
  }
  else if (c == UMaterialExpressionAntialiasedTextureMask::StaticClassName())
  {
    result = new UMaterialExpressionAntialiasedTextureMask(exp);
  }
  else if (c == UMaterialExpressionTextureSampleParameterMeshSubUV::StaticClassName())
  {
    result = new UMaterialExpressionTextureSampleParameterMeshSubUV(exp);
  }
  else if (c == UMaterialExpressionTextureSampleParameterMeshSubUVBlend::StaticClassName())
  {
    result = new UMaterialExpressionTextureSampleParameterMeshSubUVBlend(exp);
  }
  else if (c == UMaterialExpressionTextureSampleParameterSubUV::StaticClassName())
  {
    result = new UMaterialExpressionTextureSampleParameterSubUV(exp);
  }
  else if (c == UMaterialExpressionTextureSampleParameterCube::StaticClassName())
  {
    result = new UMaterialExpressionTextureSampleParameterCube(exp);
  }
  else if (c == UMaterialExpressionTextureSampleParameterMovie::StaticClassName())
  {
    result = new UMaterialExpressionTextureSampleParameterMovie(exp);
  }
  else if (c == UMaterialExpressionTextureSampleParameterNormal::StaticClassName())
  {
    result = new UMaterialExpressionTextureSampleParameterNormal(exp);
  }
  else if (c == UMaterialExpressionTime::StaticClassName())
  {
    result = new UMaterialExpressionTime(exp);
  }
  else if (c == UMaterialExpressionTransform::StaticClassName())
  {
    result = new UMaterialExpressionTransform(exp);
  }
  else if (c == UMaterialExpressionTransformPosition::StaticClassName())
  {
    result = new UMaterialExpressionTransformPosition(exp);
  }
  else if (c == UMaterialExpressionTwoSidedSign::StaticClassName())
  {
    result = new UMaterialExpressionTwoSidedSign(exp);
  }
  else if (c == UMaterialExpressionVertexColor::StaticClassName())
  {
    result = new UMaterialExpressionVertexColor(exp);
  }
  else if (c == UMaterialExpressionWindDirectionAndSpeed::StaticClassName())
  {
    result = new UMaterialExpressionWindDirectionAndSpeed(exp);
  }
  else if (c == UMaterialExpressionWorldNormal::StaticClassName())
  {
    result = new UMaterialExpressionWorldNormal(exp);
  }
  else if (c == UMaterialExpressionWorldPosition::StaticClassName())
  {
    result = new UMaterialExpressionWorldPosition(exp);
  }
  else
  {
    result = new UMaterialExpression(exp);
  }
  return result;
}

bool UMaterialExpression::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INT_PROP(MaterialExpressionEditorX);
  REGISTER_INT_PROP(MaterialExpressionEditorY);
  REGISTER_STR_PROP(Desc);
  return false;
}

FString UMaterialExpression::GetTitle() const
{
  FString title = GetClassName();
  if (title.StartWith(UMaterialExpression::StaticClassName()))
  {
    title = title.Substr(strlen(UMaterialExpression::StaticClassName()));
    if (title.Empty())
    {
      title = GetClassName();
    }
  }
  return title;
}

void UMaterialExpression::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  if (!visitor)
  {
    return;
  }
  
  visitor->SetTitle(GetTitle());
  visitor->SetEditorPosition(MaterialExpressionEditorX, MaterialExpressionEditorY);
  visitor->SetDescription(Desc);
}

bool UMaterialExpressionAbs::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionAbs::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

bool UMaterialExpressionAdd::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionAdd::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

bool UMaterialExpressionAppendVector::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionAppendVector::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

bool UMaterialExpressionBumpOffset::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinate);
  REGISTER_INPUT(Height);
  REGISTER_FLOAT_PROP(HeightRatio);
  REGISTER_FLOAT_PROP(ReferencePlane);
  return false;
}

void UMaterialExpressionBumpOffset::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinate, Height);
  visitor->SetValue(FString::Sprintf("HeightRatio: %.2f\nReferencePlane: %.2f", HeightRatio, ReferencePlane));
}

bool UMaterialExpressionCeil::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionCeil::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

bool UMaterialExpressionClamp::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_INPUT(Min);
  REGISTER_INPUT(Max);
  return false;
}

void UMaterialExpressionClamp::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input, Min, Max);
}

bool UMaterialExpressionComment::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INT_PROP(PosX);
  REGISTER_INT_PROP(PosY);
  REGISTER_INT_PROP(SizeX);
  REGISTER_INT_PROP(SizeY);
  REGISTER_STR_PROP(Text);
  return false;
}

void UMaterialExpressionComment::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetEditorSize(SizeX, SizeY);
  visitor->SetValue(Text);
}

bool UMaterialExpressionComponentMask::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_BOOL_PROP(R);
  REGISTER_BOOL_PROP(G);
  REGISTER_BOOL_PROP(B);
  REGISTER_BOOL_PROP(A);
  return false;
}

void UMaterialExpressionComponentMask::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  std::vector<char> channels;
  if (R)
  {
    channels.push_back('R');
  }
  if (G)
  {
    channels.push_back('G');
  }
  if (B)
  {
    channels.push_back('B');
  }
  if (A)
  {
    channels.push_back('A');
  }
  std::string mask;
  for (const auto& piece : channels)
  {
    mask.empty() ? mask += piece : mask += std::string(",") + piece;
  }
  visitor->SetValue(mask);
}

bool UMaterialExpressionCompound::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, MaterialExpressions))
  {
    MaterialExpressionsProperty = property;
    for (FPropertyValue* value : property->GetArray())
    {
      MaterialExpressions.push_back(Cast<UMaterialExpression>(value->GetObjectValuePtr()));
    }
    return true;
  }
  REGISTER_STR_PROP(Caption);
  REGISTER_BOOL_PROP(bExpanded);
  return false;
}

void UMaterialExpressionCompound::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  FString expressions;
  for (const auto exp : MaterialExpressions)
  {
    expressions.Empty() ? expressions += exp->GetObjectName() : expressions += FString(":") + exp->GetObjectName();
  }
  visitor->SetValue(expressions);
  visitor->SetTitle(GetTitle() + ": " + Caption);
}

bool UMaterialExpressionConstant::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(R);
  return false;
}

void UMaterialExpressionConstant::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("%.2f", R));
}

bool UMaterialExpressionConstant2Vector::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(R);
  REGISTER_FLOAT_PROP(G);
  return false;
}

void UMaterialExpressionConstant2Vector::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("%.2f, %.2f", R, G));
}

bool UMaterialExpressionConstant3Vector::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(R);
  REGISTER_FLOAT_PROP(G);
  REGISTER_FLOAT_PROP(B);
  return false;
}

void UMaterialExpressionConstant3Vector::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("%.2f, %.2f, %.2f", R, G, B));
}

bool UMaterialExpressionConstant4Vector::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(R);
  REGISTER_FLOAT_PROP(G);
  REGISTER_FLOAT_PROP(B);
  REGISTER_FLOAT_PROP(A);
  return false;
}

void UMaterialExpressionConstant4Vector::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("%.2f, %.2f, %.2f, %.2f", R, G, B, A));
}

bool UMaterialExpressionConstantBiasScale::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_FLOAT_PROP(Bias);
  REGISTER_FLOAT_PROP(Scale);
  return false;
}

void UMaterialExpressionConstantBiasScale::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor->SetValue(FString::Sprintf("Bias: %.2f\nScale: %.2f", Bias, Scale));
}

bool UMaterialExpressionConstantClamp::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_FLOAT_PROP(Min);
  REGISTER_FLOAT_PROP(Max);
  return false;
}

void UMaterialExpressionConstantClamp::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor->SetValue(FString::Sprintf("Min: %.2f\nMax: %.2f", Min, Max));
}

bool UMaterialExpressionCosine::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_FLOAT_PROP(Period);
  return false;
}

void UMaterialExpressionCosine::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor->SetValue(FString::Sprintf("Period: %.2f", Period));
}

bool UMaterialExpressionCrossProduct::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionCrossProduct::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

bool UMaterialExpressionCustomTexture::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_OBJ_PROP(Texture);
  return false;
}

void UMaterialExpressionCustomTexture::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(Texture ? Texture->GetObjectName() : "NULL");
}

bool UMaterialExpressionDepthBiasedAlpha::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bNormalize);
  REGISTER_FLOAT_PROP(BiasScale);
  REGISTER_INPUT(Alpha);
  REGISTER_INPUT(Bias);
  return false;
}

void UMaterialExpressionDepthBiasedAlpha::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Alpha, Bias);
  visitor->SetValue(FString::Sprintf("bNormalize: %s\nBiasScale: %.2f", bNormalize ? "true" : "false", BiasScale));
}

bool UMaterialExpressionDepthBiasedBlend::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bNormalize);
  REGISTER_FLOAT_PROP(BiasScale);
  REGISTER_INPUT(RGB);
  REGISTER_INPUT(Alpha);
  REGISTER_INPUT(Bias);
  return false;
}

void UMaterialExpressionDepthBiasedBlend::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(RGB, Alpha, Bias);
  visitor->SetValue(FString::Sprintf("bNormalize: %s\nBiasScale: %.2f", bNormalize ? "true" : "false", BiasScale));
}

bool UMaterialExpressionDepthOfFieldFunction::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BYTE_PROP(FunctionValue);
  REGISTER_INPUT(Depth);
  return false;
}

void UMaterialExpressionDepthOfFieldFunction::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Depth);
  visitor->SetValue(FString::Sprintf("FunctionValue: %d", int(FunctionValue)));
}

bool UMaterialExpressionDeriveNormalZ::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(InXY);
  return false;
}

void UMaterialExpressionDeriveNormalZ::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(InXY);
}

bool UMaterialExpressionDesaturation::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_INPUT(Percent);
  REGISTER_LCOL_PROP(LuminanceFactors);
  return false;
}

void UMaterialExpressionDesaturation::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input, Percent);
  visitor->SetValue(FString::Sprintf("LumFactors:\nR: %.2f G: %.2f\nB: %.2f A: %.2f", LuminanceFactors.R, LuminanceFactors.G, LuminanceFactors.B, LuminanceFactors.A));
}

bool UMaterialExpressionDestDepth::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bNormalize);
  return false;
}

void UMaterialExpressionDestDepth::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("bNormalize: %s", bNormalize ? "true" : "false"));
}

bool UMaterialExpressionDistance::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionDistance::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

bool UMaterialExpressionDivide::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionDivide::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

bool UMaterialExpressionDotProduct::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionDotProduct::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

bool UMaterialExpressionDynamicParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, ParamNames))
  {
    ParamNamesProperty = property;
    for (const auto& param : property->GetArray())
    {
      ParamNames.push_back(param->GetString());
    }
    return true;
  }
  return false;
}

void UMaterialExpressionDynamicParameter::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  FString v;
  for (const auto& name : ParamNames)
  {
    v.Empty() ? v += name : v += FString(",") + name;
  }
  visitor->SetValue(v);
}

bool UMaterialExpressionFloor::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionFloor::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

bool UMaterialExpressionFluidNormal::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinates);
  return false;
}

void UMaterialExpressionFluidNormal::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinates);
}

bool UMaterialExpressionFmod::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionFmod::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

bool UMaterialExpressionFontSample::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_OBJ_PROP(Font);
  REGISTER_INT_PROP(FontTexturePage);
  return false;
}

void UMaterialExpressionFontSample::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("Font: %s\nFontTexturePage: %d", (Font ? Font->GetObjectName().UTF8().c_str() : "NULL"), FontTexturePage));
}

bool UMaterialExpressionFontSampleParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_NAME_PROP(ParameterName);
  return false;
}

void UMaterialExpressionFontSampleParameter::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(ParameterName.String());
}

bool UMaterialExpressionFrac::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionFrac::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

bool UMaterialExpressionFresnel::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(Exponent);
  REGISTER_INPUT(Normal);
  return false;
}

void UMaterialExpressionFresnel::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Normal);
  visitor->SetValue(FString::Sprintf("Exp: %.2f", Exponent));
}

bool UMaterialExpressionIf::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  REGISTER_INPUT(AGreaterThanB);
  REGISTER_INPUT(AEqualsB);
  REGISTER_INPUT(ALessThanB);
  return false;
}

void UMaterialExpressionIf::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B, AGreaterThanB, AEqualsB, ALessThanB);
}

bool UMaterialExpressionLightmassReplace::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Realtime);
  REGISTER_INPUT(Lightmass);
  return false;
}

void UMaterialExpressionLightmassReplace::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Realtime, Lightmass);
}

bool UMaterialExpressionLinearInterpolate::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  REGISTER_INPUT(Alpha);
  return false;
}

void UMaterialExpressionLinearInterpolate::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B, Alpha);
}

bool UMaterialExpressionMultiply::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionMultiply::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

bool UMaterialExpressionNormalize::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(VectorInput);
  return false;
}

void UMaterialExpressionNormalize::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(VectorInput);
}

bool UMaterialExpressionOneMinus::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionOneMinus::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

bool UMaterialExpressionPanner::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinate);
  REGISTER_INPUT(Time);
  REGISTER_FLOAT_PROP(SpeedX);
  REGISTER_FLOAT_PROP(SpeedY);
  return false;
}

void UMaterialExpressionPanner::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinate, Time);
  visitor->SetValue(FString::Sprintf("SpdX: %.2f\nSpdY: %.2f", SpeedX, SpeedY));
}

bool UMaterialExpressionParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_NAME_PROP(ParameterName);
  return false;
}

void UMaterialExpressionParameter::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(ParameterName.String());
}

bool UMaterialExpressionScalarParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(DefaultValue);
  return false;
}

void UMaterialExpressionScalarParameter::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(ParameterName.String() + FString(":") + FString::Sprintf("%.2f", DefaultValue));
}

bool UMaterialExpressionStaticComponentMaskParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_BOOL_PROP(DefaultR);
  REGISTER_BOOL_PROP(DefaultG);
  REGISTER_BOOL_PROP(DefaultB);
  REGISTER_BOOL_PROP(DefaultA);
  return false;
}

void UMaterialExpressionStaticComponentMaskParameter::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  FString mask;
  if (DefaultR)
  {
    mask += "R";
  }
  if (DefaultG)
  {
    mask += "G";
  }
  if (DefaultB)
  {
    mask += "B";
  }
  if (DefaultA)
  {
    mask += "A";
  }
  visitor->SetValue(mask);
}

bool UMaterialExpressionStaticSwitchParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(DefaultValue);
  REGISTER_BOOL_PROP(ExtendedCaptionDisplay);
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionStaticSwitchParameter::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
  visitor->SetValue(ParameterName.String() + FString(":") + (DefaultValue ? "true" : "false"));
}

bool UMaterialExpressionVectorParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_LCOL_PROP(DefaultValue);
  return false;
}

void UMaterialExpressionVectorParameter::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(ParameterName.String() + FString::Sprintf("\n%.2f, %.2f, %.2f, %.2f", DefaultValue.R, DefaultValue.G, DefaultValue.B, DefaultValue.A));
}

bool UMaterialExpressionParticleMacroUV::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bUseViewSpace);
  return false;
}

void UMaterialExpressionParticleMacroUV::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("bUseViewSpace: %s", bUseViewSpace ? "true" : "false"));
}

bool UMaterialExpressionPixelDepth::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bNormalize);
  return false;
}

void UMaterialExpressionPixelDepth::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("bNormalize: %s", bNormalize ? "true" : "false"));
}

bool UMaterialExpressionPower::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Base);
  REGISTER_INPUT(Exponent);
  return false;
}

void UMaterialExpressionPower::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Base, Exponent);
}

bool UMaterialExpressionRotateAboutAxis::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(NormalizedRotationAxisAndAngle);
  REGISTER_INPUT(PositionOnAxis);
  REGISTER_INPUT(Position);
  return false;
}

void UMaterialExpressionRotateAboutAxis::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(NormalizedRotationAxisAndAngle, PositionOnAxis, Position);
}

bool UMaterialExpressionRotator::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinate);
  REGISTER_INPUT(Time);
  REGISTER_FLOAT_PROP(CenterX);
  REGISTER_FLOAT_PROP(CenterY);
  REGISTER_FLOAT_PROP(Speed);
  return false;
}

void UMaterialExpressionRotator::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinate, Time);
  visitor->SetValue(FString::Sprintf("Center: %.2f, %.2f\nSpeed: %.2f", CenterX, CenterY, Speed));
}

bool UMaterialExpressionSceneDepth::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinates);
  REGISTER_BOOL_PROP(bNormalize);
  return false;
}

void UMaterialExpressionSceneDepth::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("bNormalize: %s", bNormalize ? "true" : "false"));
}

bool UMaterialExpressionSceneTexture::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinates);
  REGISTER_BYTE_PROP(SceneTextureType);
  REGISTER_BOOL_PROP(ScreenAlign);
  return false;
}

void UMaterialExpressionSceneTexture::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinates);
  visitor->SetValue(FString::Sprintf("SceneTextureType: %d\nScreenAlign: %s", int(SceneTextureType), ScreenAlign ? "true" : "false"));
}

bool UMaterialExpressionScreenPosition::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(ScreenAlign);
  return false;
}

void UMaterialExpressionScreenPosition::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("ScreenAlign: %s", ScreenAlign ? "true" : "false"));
}

bool UMaterialExpressionSine::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_FLOAT_PROP(Period);
  return false;
}

void UMaterialExpressionSine::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor->SetValue(FString::Sprintf("Period: %.2f", Period));
}

bool UMaterialExpressionSphereMask::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  REGISTER_FLOAT_PROP(AttenuationRadius);
  REGISTER_FLOAT_PROP(HardnessPercent);
  return false;
}

void UMaterialExpressionSphereMask::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
  visitor->SetValue(FString::Sprintf("AttRadius: %.2f\nHrdsPercent: %.2f", AttenuationRadius, HardnessPercent));
}

bool UMaterialExpressionSquareRoot::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionSquareRoot::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

bool UMaterialExpressionSubtract::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionSubtract::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

bool UMaterialExpressionTerrainLayerCoords::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_ENUM_STR_PROP(MappingType);
  REGISTER_FLOAT_PROP(MappingScale);
  REGISTER_FLOAT_PROP(MappingRotation);
  REGISTER_FLOAT_PROP(MappingPanU);
  REGISTER_FLOAT_PROP(MappingPanV);
  return false;
}

void UMaterialExpressionTerrainLayerCoords::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("Type: %s\nScale: %.2f\nRotation: %.2f\nPan: %.2f, %.2f", MappingType.UTF8().c_str(), MappingScale, MappingRotation, MappingPanU, MappingPanV));
}

bool UMaterialExpressionTerrainLayerWeight::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Base);
  REGISTER_INPUT(Layer);
  REGISTER_NAME_PROP(ParameterName);
  return false;
}

void UMaterialExpressionTerrainLayerWeight::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Base, Layer);
  visitor->SetValue(ParameterName.String());
}

bool UMaterialExpressionTextureCoordinate::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INT_PROP(CoordinateIndex);
  REGISTER_FLOAT_PROP(UTiling);
  REGISTER_FLOAT_PROP(VTiling);
  REGISTER_BOOL_PROP(UnMirrorU);
  REGISTER_BOOL_PROP(UnMirrorV);
  return false;
}

void UMaterialExpressionTextureCoordinate::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("CoordIdx: %d\nTilingUV: %.2f, %.2f\nUnMirrorUV: %s, %s", CoordinateIndex, UTiling, VTiling, UnMirrorU ? "true" : "false", UnMirrorV ? "true" : "false"));
}

bool UMaterialExpressionTextureSample::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinates);
  REGISTER_OBJ_PROP(Texture);
  return false;
}

void UMaterialExpressionTextureSample::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinates);
  visitor->SetValue(Texture ? Texture->GetObjectName() : "NULL");
}

bool UMaterialExpressionDepthBiasBlend::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Bias);
  REGISTER_BOOL_PROP(bNormalize);
  REGISTER_FLOAT_PROP(BiasScale);
  return false;
}

void UMaterialExpressionDepthBiasBlend::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Bias);
  visitor->SetValue(FString::Sprintf("bNormalize: %s\nBiasScale: %.2f", bNormalize ? "true" : "false", BiasScale));
}

bool UMaterialExpressionAntialiasedTextureMask::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(Threshold);
  REGISTER_ENUM_STR_PROP(Channel);
  return false;
}

void UMaterialExpressionAntialiasedTextureMask::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("Threshold: %.2f\nChannel: %s", Threshold, Channel.UTF8().c_str()));
}

bool UMaterialExpressionTime::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bIgnorePause);
  return false;
}

void UMaterialExpressionTime::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(FString::Sprintf("bIgnorePause: %s", bIgnorePause ? "true" : "false"));
}

bool UMaterialExpressionTransform::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_ENUM_STR_PROP(TransformSourceType);
  REGISTER_ENUM_STR_PROP(TransformType);
  return false;
}

void UMaterialExpressionTransform::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor->SetValue(FString::Sprintf("SourceType: %s\nType: %s", TransformSourceType.UTF8().c_str(), TransformType.UTF8().c_str()));
}

bool UMaterialExpressionTransformPosition::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_ENUM_STR_PROP(TransformType);
  return false;
}

void UMaterialExpressionTransformPosition::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor->SetValue(FString::Sprintf("Type: %s", TransformType.UTF8().c_str()));
}

bool UMaterialExpressionTextureSampleParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_NAME_PROP(ParameterName);
  return false;
}

void UMaterialExpressionTextureSampleParameter::AcceptVisitor(UMaterialExpressionViewVisitor* visitor)
{
  SUPER_ACCEPT();
  visitor->SetValue(ParameterName.String());
}