#pragma once

// Glue for URefs. Don't use.
#define __GLUE_OBJ_REF(Name, Sfx)\
  Name##Sfx

// Serialize UObject* with its index
// In case we fail to find the object, we won't lose its index when saving
#define SERIALIZE_UREF(Stream, Field)\
  Stream.SerializeObjectRef((void*&)Field, __GLUE_OBJ_REF(Field, RefIndex))

// Declare an object with its index
#define DECL_UREF(TClass, Name)\
  TClass* Name = nullptr;\
  PACKAGE_INDEX __GLUE_OBJ_REF(Name, RefIndex) = 0

// Common UObject subclass declarations
#define DECL_UOBJ(TClass, TSuper)\
public:\
  inline friend FStream& operator<<(FStream& s, TClass*& obj) { return s << *(UObject**)&obj; }\
  typedef TClass ThisClass;\
  typedef TSuper Super;\
  static const char* StaticClassName() { return ((char*)#TClass) + 1; }\
  const char* GetStaticClassName() const override { return ThisClass::StaticClassName(); }\
  std::string GetStaticClassChain() const override { return Super::GetStaticClassChain() + "." + ThisClass::StaticClassName(); }\
  using TSuper::TSuper

// Common property helpers for faster property definition and registration

// Private macro for naming
#define __GLUE_PROP(TName, Suffix) TName##Suffix

// Check if the PropertyTag matches TName
#define PROP_IS(prop, TName) (prop->Name == P_##TName)

// Basic property declaration
#define UPROP(TType, TName, TDefault)\
TType TName = TDefault;\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) = nullptr

// Basic property declaration with a default value
#define UPROP_NOINIT(TType, TName)\
TType TName;\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) = nullptr

// Property with a custom method to create it
#define UPROP_CREATABLE(TType, TName, TDefault)\
TType TName = TDefault;\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) = nullptr;\
FPropertyTag* CreateProperty##TName(const TType& value)\
{\
  if (FPropertyTag* tag = CreateProperty(P_##TName))\
  {\
    tag->GetTypedValue<TType>() = value;\
    if (tag->Type == NAME_BoolProperty && tag->Value && tag->Value->Type == FPropertyValue::VID::Bool)\
    {\
      /* GetTypedValue<bool> returns tag's bool field while a property grid uses FPropertyValue to read bool values. Set correct value to FPropertyValue object here. */\
      tag->Value->GetBool() = value;\
    }\
    AddProperty(tag);\
    RegisterProperty(tag);\
    return tag;\
  }\
  return nullptr;\
}\
//

// Enum property with a custom method to create it
#define UPROP_CREATABLE_ENUM(TType, TName, TDefault)\
TType TName = TDefault;\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) = nullptr;\
FPropertyTag* CreateProperty##TName(const TType& value)\
{\
  if (FPropertyTag* tag = CreateProperty(P_##TName))\
  {\
    tag->Value->GetByte() = (uint8)value;\
    AddProperty(tag);\
    tag->Value->RegisterEnumNames();\
    RegisterProperty(tag);\
    return tag;\
  }\
  return nullptr;\
}\
//

// Static array property with a custom method to create it
#define UPROP_CREATABLE_STATIC_ARR(TType, TCount, TName, ...)\
TType TName [ TCount ] = { __VA_ARGS__ };\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) [ TCount ] = { nullptr };\
FPropertyTag* CreateProperty##TName(const TType& value, int32 arrayIdx)\
{\
  if (FPropertyTag* tag = CreateProperty(P_##TName))\
  {\
    tag->Value->GetTypedValue<TType>() = value;\
    if (arrayIdx &&  __GLUE_PROP(TName, Property)[arrayIdx - 1])\
    {\
      __GLUE_PROP(TName, Property)[arrayIdx - 1]->StaticArrayNext = tag;\
    }\
    else if (arrayIdx + 1 < TCount && __GLUE_PROP(TName, Property)[arrayIdx + 1])\
    {\
      tag->StaticArrayNext = __GLUE_PROP(TName, Property)[arrayIdx - 1];\
    }\
    tag->ArrayIndex = arrayIdx;\
    AddProperty(tag);\
    RegisterProperty(tag);\
    return tag;\
  }\
  return nullptr;\
}\
//

// Array property with a custom method to create it
#define UPROP_CREATABLE_ARR_PTR(TName)\
std::vector<FPropertyValue*>* TName = nullptr;\
const char* P_##TName = #TName;\
FPropertyTag* __GLUE_PROP(TName, Property) = nullptr;\
FPropertyTag* CreateProperty##TName()\
{\
  if (FPropertyTag* tag = CreateProperty(P_##TName))\
  {\
    tag->Value->GetArray() = *(new std::vector<FPropertyValue*>);\
    AddProperty(tag);\
    RegisterProperty(tag);\
    return tag;\
  }\
  return nullptr;\
}
//

// Private registration macros

#define __REGISTER_PROP(TName, TType)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  TName = property->__GLUE_PROP(Get,TType)();\
  return true;\
}\
//

#define __REGISTER_TYPED_PROP(TName, TCast, TType)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  TName = (TCast)property->__GLUE_PROP(Get,TType)();\
  return true;\
}\
//

#define REGISTER_ENUM_STR_PROP(TName)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  if (property->Value->Enum)\
  {\
    TName = property->Value->Enum->GetEnum(property->GetByte()).String().String();\
  }\
  else\
  {\
    TName = FString::Sprintf("%d", int(property->GetByte()));\
  }\
  return true;\
}\
//

// Public registration macros

#define REGISTER_FLOAT_PROP(TName) __REGISTER_PROP(TName, Float)

#define REGISTER_INT_PROP(TName) __REGISTER_PROP(TName, Int)

#define REGISTER_BOOL_PROP(TName) __REGISTER_PROP(TName, Bool)

#define REGISTER_BYTE_PROP(TName) __REGISTER_PROP(TName, Byte)

#define REGISTER_STR_PROP(TName) __REGISTER_PROP(TName, String)

#define REGISTER_OBJ_PROP(TName) __REGISTER_PROP(TName, ObjectValuePtr)

#define REGISTER_TOBJ_PROP(TName, TType) __REGISTER_TYPED_PROP(TName, TType, ObjectValuePtr)

#define REGISTER_NAME_PROP(TName) __REGISTER_PROP(TName, Name)

#define REGISTER_VEC_PROP(TName)\
if (PROP_IS(property, TName))\
{\
__GLUE_PROP(TName, Property) = property; \
return property->GetVector(TName); \
}\
//

#define REGISTER_ROT_PROP(TName)\
if (PROP_IS(property, TName))\
{\
__GLUE_PROP(TName, Property) = property; \
return property->GetRotator(TName); \
}\
//

#define REGISTER_COL_PROP(TName)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  property->GetColor(TName);\
  return true;\
}\
//

#define REGISTER_LCOL_PROP(TName)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  property->GetLinearColor(TName);\
  return true;\
}\
//

#define SUPER_REGISTER_PROP()\
if (Super::RegisterProperty(property))\
{\
  return true;\
}\
//