#pragma once
#include "UMaterial.h"

struct FExpressionInput {
  FString Title;
  class UMaterialExpression* Expression = nullptr;
  bool Mask = false;
  bool MaskR = false;
  bool MaskG = false;
  bool MaskB = false;
  bool MaskA = false;

  FExpressionInput() = default;

  FExpressionInput(FPropertyTag* property);

  FString GetDescription() const;

  bool IsConnected() const
  {
    return Expression;
  }
};

class UMaterialExpressionViewVisitor {
public:
  virtual ~UMaterialExpressionViewVisitor()
  {}

  virtual void SetTitle(const FString& title) = 0;
  virtual void SetValue(const FString& value) = 0;
  virtual void SetDescription(const FString& desc) = 0;
  virtual void SetInput(const std::vector<FExpressionInput>& input) = 0;
  virtual void SetEditorPosition(int32 x, int32 y) = 0;
  virtual void SetEditorSize(int32 x, int32 y) = 0;
};



class UMaterialExpression : public UObject {
public:
  DECL_UOBJ(UMaterialExpression, UObject);
  UPROP(int32, MaterialExpressionEditorX, 0);
  UPROP(int32, MaterialExpressionEditorY, 0);
  UPROP(int32, EditorX, 0);
  UPROP(int32, EditorY, 0);
  UPROP_NOINIT(FString, Desc);

  int32 GetPosX() const;
  int32 GetPosY() const;

  static UMaterialExpression* StaticFactory(FObjectExport* exp);

  bool RegisterProperty(FPropertyTag* property) override;
  FString GetTitle() const;
  virtual void AcceptVisitor(UMaterialExpressionViewVisitor* visitor);
};

class UMaterialExpressionAbs : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionAbs, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionAdd : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionAdd, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionAppendVector : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionAppendVector, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionBumpOffset : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionBumpOffset, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Coordinate);
  UPROP_NOINIT(FExpressionInput, Height);
  UPROP(float, HeightRatio, 0.);
  UPROP(float, ReferencePlane, 0.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionCameraVector : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionCameraVector, UMaterialExpression);
};

class UMaterialExpressionCameraWorldPosition : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionCameraWorldPosition, UMaterialExpression);
};

class UMaterialExpressionCeil : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionCeil, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionClamp : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionClamp, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  UPROP_NOINIT(FExpressionInput, Min);
  UPROP_NOINIT(FExpressionInput, Max);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionComment : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionComment, UMaterialExpression);
  UPROP(int32, PosX, 0);
  UPROP(int32, PosY, 0);
  UPROP(int32, SizeX, 0);
  UPROP(int32, SizeY, 0);
  UPROP_NOINIT(FString, Text);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionComponentMask : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionComponentMask, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  UPROP(bool, R, false);
  UPROP(bool, G, false);
  UPROP(bool, B, false);
  UPROP(bool, A, false);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionCompound : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionCompound, UMaterialExpression);
  UPROP_NOINIT(std::vector<UMaterialExpression*>, MaterialExpressions);
  UPROP_NOINIT(FString, Caption);
  UPROP(bool, bExpanded, false);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionConstant : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionConstant, UMaterialExpression);
  UPROP(float, R, 0.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionConstant2Vector : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionConstant2Vector, UMaterialExpression);
  UPROP(float, R, 0.);
  UPROP(float, G, 0.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionConstant3Vector : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionConstant3Vector, UMaterialExpression);
  UPROP(float, R, 0.);
  UPROP(float, G, 0.);
  UPROP(float, B, 0.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionConstant4Vector : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionConstant4Vector, UMaterialExpression);
  UPROP(float, R, 0.);
  UPROP(float, G, 0.);
  UPROP(float, B, 0.);
  UPROP(float, A, 0.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionConstantBiasScale : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionConstantBiasScale, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  UPROP(float, Bias, 1.);
  UPROP(float, Scale, .5);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionConstantClamp : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionConstantClamp, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  UPROP(float, Min, 0.);
  UPROP(float, Max, 1.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionCosine : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionCosine, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  UPROP(float, Period, 1.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionCrossProduct : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionCrossProduct, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionCustomTexture : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionCustomTexture, UMaterialExpression);
  UPROP(UObject*, Texture, nullptr);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDepthBiasedAlpha : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDepthBiasedAlpha, UMaterialExpression);
  UPROP(bool, bNormalize, false);
  UPROP(float, BiasScale, 1.);
  UPROP_NOINIT(FExpressionInput, Alpha);
  UPROP_NOINIT(FExpressionInput, Bias);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDepthBiasedBlend : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDepthBiasedBlend, UMaterialExpression);
  UPROP(bool, bNormalize, false);
  UPROP(float, BiasScale, 1.);
  UPROP_NOINIT(FExpressionInput, RGB);
  UPROP_NOINIT(FExpressionInput, Alpha);
  UPROP_NOINIT(FExpressionInput, Bias);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDepthOfFieldFunction : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDepthOfFieldFunction, UMaterialExpression);
  UPROP(uint8, FunctionValue, 0);
  UPROP_NOINIT(FExpressionInput, Depth);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDeriveNormalZ : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDeriveNormalZ, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, InXY);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDesaturation : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDesaturation, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  UPROP_NOINIT(FExpressionInput, Percent);
  UPROP(FLinearColor, LuminanceFactors, FLinearColor(.3f, .59f, .11f, 1.f));
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDestColor : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDestColor, UMaterialExpression);
};

class UMaterialExpressionDestDepth : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDestDepth, UMaterialExpression);
  UPROP(bool, bNormalize, false);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDistance : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDistance, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDivide : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDivide, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDotProduct : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDotProduct, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDynamicParameter : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionDynamicParameter, UMaterialExpression);
  UPROP_NOINIT(std::vector<FString>, ParamNames);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionMeshEmitterDynamicParameter : public UMaterialExpressionDynamicParameter {
public:
  DECL_UOBJ(UMaterialExpressionMeshEmitterDynamicParameter, UMaterialExpressionDynamicParameter);
};

class UMaterialExpressionFloor : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionFloor, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionFluidNormal : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionFluidNormal, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Coordinates);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionFmod : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionFmod, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionFoliageImpulseDirection : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionFoliageImpulseDirection, UMaterialExpression);
};

class UMaterialExpressionFoliageNormalizedRotationAxisAndAngle : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionFoliageNormalizedRotationAxisAndAngle, UMaterialExpression);
};

class UMaterialExpressionFontSample : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionFontSample, UMaterialExpression);
  UPROP(UObject*, Font, nullptr);
  UPROP(int32, FontTexturePage, 0);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionFontSampleParameter : public UMaterialExpressionFontSample {
public:
  DECL_UOBJ(UMaterialExpressionFontSampleParameter, UMaterialExpressionFontSample);
  UPROP_NOINIT(FName, ParameterName);
  UPROP_NOINIT(FGuid, ExpressionGUID);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionFrac : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionFrac, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionFresnel : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionFresnel, UMaterialExpression);
  UPROP(float, Exponent, 3.);
  UPROP_NOINIT(FExpressionInput, Normal);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionIf : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionIf, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  UPROP_NOINIT(FExpressionInput, AGreaterThanB);
  UPROP_NOINIT(FExpressionInput, AEqualsB);
  UPROP_NOINIT(FExpressionInput, ALessThanB);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionLensFlareIntensity : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionLensFlareIntensity, UMaterialExpression);
};

class UMaterialExpressionLensFlareOcclusion : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionLensFlareOcclusion, UMaterialExpression);
};

class UMaterialExpressionLensFlareRadialDistance : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionLensFlareRadialDistance, UMaterialExpression);
};

class UMaterialExpressionLensFlareRayDistance : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionLensFlareRayDistance, UMaterialExpression);
};

class UMaterialExpressionLensFlareSourceDistance : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionLensFlareSourceDistance, UMaterialExpression);
};

class UMaterialExpressionLightmapUVs : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionLightmapUVs, UMaterialExpression);
};

class UMaterialExpressionLightmassReplace : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionLightmassReplace, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Realtime);
  UPROP_NOINIT(FExpressionInput, Lightmass);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionLightVector : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionLightVector, UMaterialExpression);
};

class UMaterialExpressionLinearInterpolate : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionLinearInterpolate, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  UPROP_NOINIT(FExpressionInput, Alpha);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionMeshEmitterVertexColor : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionMeshEmitterVertexColor, UMaterialExpression);
};

class UMaterialExpressionMultiply : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionMultiply, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionNormalize : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionNormalize, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, VectorInput);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionObjectOrientation : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionObjectOrientation, UMaterialExpression);
};

class UMaterialExpressionObjectRadius : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionObjectRadius, UMaterialExpression);
};

class UMaterialExpressionObjectWorldPosition : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionObjectWorldPosition, UMaterialExpression);
};

class UMaterialExpressionOcclusionPercentage : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionOcclusionPercentage, UMaterialExpression);
};

class UMaterialExpressionOneMinus : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionOneMinus, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionPanner : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionPanner, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Coordinate);
  UPROP_NOINIT(FExpressionInput, Time);
  UPROP(float, SpeedX, 0.);
  UPROP(float, SpeedY, 0.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionParameter : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionParameter, UMaterialExpression);
  UPROP_NOINIT(FName, ParameterName);
  UPROP_NOINIT(FGuid, ExpressionGUID);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionScalarParameter : public UMaterialExpressionParameter {
public:
  DECL_UOBJ(UMaterialExpressionScalarParameter, UMaterialExpressionParameter);
  UPROP(float, DefaultValue, 0.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionStaticComponentMaskParameter : public UMaterialExpressionParameter {
public:
  DECL_UOBJ(UMaterialExpressionStaticComponentMaskParameter, UMaterialExpressionParameter);
  UPROP_NOINIT(FExpressionInput, Input);
  UPROP(bool, DefaultR, false);
  UPROP(bool, DefaultG, false);
  UPROP(bool, DefaultB, false);
  UPROP(bool, DefaultA, false);
  FStaticComponentMaskParameter* InstanceOverride = nullptr;
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionStaticSwitchParameter : public UMaterialExpressionParameter {
public:
  DECL_UOBJ(UMaterialExpressionStaticSwitchParameter, UMaterialExpressionParameter);
  UPROP(bool, DefaultValue, false);
  UPROP(bool, ExtendedCaptionDisplay, false);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  FStaticSwitchParameter* InstanceOverride = nullptr;
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionVectorParameter : public UMaterialExpressionParameter {
public:
  DECL_UOBJ(UMaterialExpressionVectorParameter, UMaterialExpressionParameter);
  UPROP_NOINIT(FLinearColor, DefaultValue);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionParticleMacroUV : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionParticleMacroUV, UMaterialExpression);
  UPROP(bool, bUseViewSpace, false);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionPerInstanceRandom : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionPerInstanceRandom, UMaterialExpression);
};

class UMaterialExpressionPixelDepth : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionPixelDepth, UMaterialExpression);
  UPROP(bool, bNormalize, false);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionPower : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionPower, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Base);
  UPROP_NOINIT(FExpressionInput, Exponent);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionReflectionVector : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionReflectionVector, UMaterialExpression);
};

class UMaterialExpressionRotateAboutAxis : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionRotateAboutAxis, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, NormalizedRotationAxisAndAngle);
  UPROP_NOINIT(FExpressionInput, PositionOnAxis);
  UPROP_NOINIT(FExpressionInput, Position);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionRotator : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionRotator, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Coordinate);
  UPROP_NOINIT(FExpressionInput, Time);
  UPROP(float, CenterX, 0.5);
  UPROP(float, CenterY, 0.5);
  UPROP(float, Speed, 0.25);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionSceneDepth : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionSceneDepth, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Coordinates);
  UPROP(bool, bNormalize, false);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionSceneTexture : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionSceneTexture, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Coordinates);
  UPROP(uint8, SceneTextureType, 0);
  UPROP(bool, ScreenAlign, false);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionScreenPosition : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionScreenPosition, UMaterialExpression);
  UPROP(bool, ScreenAlign, false);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionSine : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionSine, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  UPROP(float, Period, 1.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionSphereMask : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionSphereMask, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  UPROP(float, AttenuationRadius, 256.);
  UPROP(float, HardnessPercent, 100.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionSquareRoot : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionSquareRoot, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionSubtract : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionSubtract, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, A);
  UPROP_NOINIT(FExpressionInput, B);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionTerrainLayerCoords : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionTerrainLayerCoords, UMaterialExpression);
  UPROP(FString, MappingType, "Auto");
  UPROP(float, MappingScale, 0.);
  UPROP(float, MappingRotation, 0.);
  UPROP(float, MappingPanU, 0.);
  UPROP(float, MappingPanV, 0.);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionTerrainLayerWeight : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionTerrainLayerWeight, UMaterialExpression);
  FStaticTerrainLayerWeightParameter* InstanceOverride = nullptr;
  UPROP_NOINIT(FExpressionInput, Base);
  UPROP_NOINIT(FExpressionInput, Layer);
  UPROP_NOINIT(FName, ParameterName);
  UPROP(float, PreviewWeight, 0.);
  UPROP_NOINIT(FGuid, ExpressionGUID);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionTextureCoordinate : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionTextureCoordinate, UMaterialExpression);
  UPROP(int32, CoordinateIndex, 0);
  UPROP(float, UTiling, 1.);
  UPROP(float, VTiling, 1.);
  UPROP(bool, UnMirrorU, false);
  UPROP(bool, UnMirrorV, false);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionTextureSample : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionTextureSample, UMaterialExpression);
  UPROP(UObject*, Texture, nullptr);
  UPROP_NOINIT(FExpressionInput, Coordinates);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionDepthBiasBlend : public UMaterialExpressionTextureSample {
public:
  DECL_UOBJ(UMaterialExpressionDepthBiasBlend, UMaterialExpressionTextureSample);
  UPROP(bool, bNormalize, false);
  UPROP(float, BiasScale, 1.);
  UPROP_NOINIT(FExpressionInput, Bias);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionFlipBookSample : public UMaterialExpressionTextureSample {
public:
  DECL_UOBJ(UMaterialExpressionFlipBookSample, UMaterialExpressionTextureSample);
};

class UMaterialExpressionMeshSubUV : public UMaterialExpressionTextureSample {
public:
  DECL_UOBJ(UMaterialExpressionMeshSubUV, UMaterialExpressionTextureSample);
};

class UMaterialExpressionMeshSubUVBlend : public UMaterialExpressionMeshSubUV {
public:
  DECL_UOBJ(UMaterialExpressionMeshSubUVBlend, UMaterialExpressionMeshSubUV);
};

class UMaterialExpressionParticleSubUV : public UMaterialExpressionTextureSample {
public:
  DECL_UOBJ(UMaterialExpressionParticleSubUV, UMaterialExpressionTextureSample);
};

class UMaterialExpressionTextureSampleParameter : public UMaterialExpressionTextureSample {
public:
  DECL_UOBJ(UMaterialExpressionTextureSampleParameter, UMaterialExpressionTextureSample);
  
  UPROP_NOINIT(FName, ParameterName);

  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionTextureSampleParameter2D : public UMaterialExpressionTextureSampleParameter {
public:
  DECL_UOBJ(UMaterialExpressionTextureSampleParameter2D, UMaterialExpressionTextureSampleParameter);
};

class UMaterialExpressionAntialiasedTextureMask : public UMaterialExpressionTextureSampleParameter2D {
public:
  DECL_UOBJ(UMaterialExpressionAntialiasedTextureMask, UMaterialExpressionTextureSampleParameter2D);
  UPROP(float, Threshold, .5);
  UPROP(FString, Channel, "Alpha");
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionTextureSampleParameterMeshSubUV : public UMaterialExpressionTextureSampleParameter2D {
public:
  DECL_UOBJ(UMaterialExpressionTextureSampleParameterMeshSubUV, UMaterialExpressionTextureSampleParameter2D);
};

class UMaterialExpressionTextureSampleParameterMeshSubUVBlend : public UMaterialExpressionTextureSampleParameterMeshSubUV {
public:
  DECL_UOBJ(UMaterialExpressionTextureSampleParameterMeshSubUVBlend, UMaterialExpressionTextureSampleParameterMeshSubUV);
};

class UMaterialExpressionTextureSampleParameterSubUV : public UMaterialExpressionTextureSampleParameter2D {
public:
  DECL_UOBJ(UMaterialExpressionTextureSampleParameterSubUV, UMaterialExpressionTextureSampleParameter2D);
};

class UMaterialExpressionTextureSampleParameterCube : public UMaterialExpressionTextureSampleParameter {
public:
  DECL_UOBJ(UMaterialExpressionTextureSampleParameterCube, UMaterialExpressionTextureSampleParameter);
};

class UMaterialExpressionTextureSampleParameterMovie : public UMaterialExpressionTextureSampleParameter {
public:
  DECL_UOBJ(UMaterialExpressionTextureSampleParameterMovie, UMaterialExpressionTextureSampleParameter);
};

class UMaterialExpressionTextureSampleParameterNormal : public UMaterialExpressionTextureSampleParameter {
public:
  DECL_UOBJ(UMaterialExpressionTextureSampleParameterNormal, UMaterialExpressionTextureSampleParameter);
  FNormalParameter* InstanceOverride = nullptr;
};

class UMaterialExpressionTime : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionTime, UMaterialExpression);
  UPROP(bool, bIgnorePause, false);
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionTransform : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionTransform, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  UPROP(FString, TransformSourceType, "Tangent");
  UPROP(FString, TransformType, "World");
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionTransformPosition : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionTransformPosition, UMaterialExpression);
  UPROP_NOINIT(FExpressionInput, Input);
  UPROP(FString, TransformType, "World");
  bool RegisterProperty(FPropertyTag* property) override;
  void AcceptVisitor(UMaterialExpressionViewVisitor* visitor) override;
};

class UMaterialExpressionTwoSidedSign : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionTwoSidedSign, UMaterialExpression);
};

class UMaterialExpressionVertexColor : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionVertexColor, UMaterialExpression);
};

class UMaterialExpressionWindDirectionAndSpeed : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionWindDirectionAndSpeed, UMaterialExpression);
};

class UMaterialExpressionWorldNormal : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionWorldNormal, UMaterialExpression);
};

class UMaterialExpressionWorldPosition : public UMaterialExpression {
public:
  DECL_UOBJ(UMaterialExpressionWorldPosition, UMaterialExpression);
};