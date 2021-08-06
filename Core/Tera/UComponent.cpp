#include "UComponent.h"
#include "FStream.h"
#include "UClass.h"

void UComponent::PreSerialize(FStream& s)
{
  SERIALIZE_UREF(s, TemplateOwnerClass);
  if (IsTemplate(RF_ClassDefaultObject))
  {
    s << TemplateName;
  }
}

float UDistributionFloat::GetFloat(float f)
{
  return GetValue(f);
}

float UDistributionFloat::GetValue(float f, UObject* data)
{
  return 0.f;
}

void UDistributionFloat::GetInRange(float& minIn, float& maxIn)
{
  minIn = 0.0f;
  maxIn = 0.0f;
}

void UDistributionFloat::GetOutRange(float& minOut, float& maxOut)
{
  minOut = 0.0f;
  maxOut = 0.0f;
}

bool UDistributionFloatUniform::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(Min);
  REGISTER_FLOAT_PROP(Max);
  return false;
}

float UDistributionFloatUniform::GetValue(float f, UObject* data)
{
  return Max + (Min - Max) * USRand();
}

void UDistributionFloatUniform::GetOutRange(float& minOut, float& maxOut)
{
  minOut = Min;
  maxOut = Max;
}

bool UDistributionFloatConstant::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(Constant);
  return false;
}

float UDistributionFloatConstant::GetValue(float f, UObject* data)
{
  return Constant;
}

void UDistributionFloatConstant::GetOutRange(float& minOut, float& maxOut)
{
  minOut = Constant;
  maxOut = Constant;
}
