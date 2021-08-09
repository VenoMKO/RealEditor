#pragma once
#include "UObject.h"
#include "FName.h"

class UComponent : public UObject {
public:
  DECL_UOBJ(UComponent, UObject);

  void PreSerialize(FStream& s);

  bool IsComponent() const override
  {
    return true;
  }

  DECL_UREF(UClass, TemplateOwnerClass);
  FName TemplateName;
};

class UDistributionFloat : public UComponent {
public:
  DECL_UOBJ(UDistributionFloat, UComponent);

  virtual bool IsValid() const
  {
    return false;
  }
  virtual float GetFloat(float f = 0.f);
  virtual float GetValue(float f = 0.f, UObject* data = nullptr);
  virtual void GetInRange(float& minIn, float& maxIn);
  virtual void GetOutRange(float& minOut, float& maxOut);
};

class UDistributionFloatConstant : public UDistributionFloat {
public:
  DECL_UOBJ(UDistributionFloatConstant, UDistributionFloat);
  UPROP(float, Constant, 0.f);

  bool RegisterProperty(FPropertyTag* property) override;

  bool IsValid() const override
  {
    return ConstantProperty;
  }

  float GetValue(float f = 0.f, UObject* data = nullptr) override;
  void GetOutRange(float& minOut, float& maxOut) override;
};

class UDistributionFloatUniform : public UDistributionFloat {
public:
  DECL_UOBJ(UDistributionFloatUniform, UDistributionFloat);

  UPROP(float, Min, 0.f);
  UPROP(float, Max, 0.f);

  bool RegisterProperty(FPropertyTag* property) override;

  bool IsValid() const override
  {
    return MinProperty && MaxProperty;
  }

  float GetValue(float f = 0.f, UObject* data = nullptr) override;
  void GetOutRange(float& minOut, float& maxOut) override;
};