#include "UProperty.h"
#include "FStream.h"
#include "FPropertyTag.h"
#include "FStructs.h"
#include "FString.h"
#include "FPackage.h"

#define CPF_Net 0x0000000000000020
#define STRUCT_Immutable 0x00000020
#define STRUCT_ImmutableWhenCooked 0x00000080

void UProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ArrayDim;
  s << PropertyFlags;
  s << Category;
  SERIALIZE_UREF(s, ArraySizeEnum);
  if (PropertyFlags & CPF_Net)
  {
    s << RepOffset;
  }
}

void UObjectProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, PropertyClass);
}

void UClassProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, MetaClass);
}

void UInterfaceProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, InterfaceClass);
}

void UArrayProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, Inner);
}

void UMapProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, Key);
  SERIALIZE_UREF(s, Value);
}

void UStructProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, Struct);
}

void UByteProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, Enum);
}

void UDelegateProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, Function);
  SERIALIZE_UREF(s, SourceDelegate);
}

void UInterfaceProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  UObject* tmp = nullptr;
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Object;
    valuePtr->Data = new PACKAGE_INDEX;
  }
  else
  {
    tmp = valuePtr->Property->Owner->GetPackage()->GetObject(valuePtr->GetObjectIndex());
  }
  s.SerializeObjectRef((void*&)tmp, valuePtr->GetObjectIndex());
}

void UMapProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  DBreak();
}

void UByteProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  const bool bUseBinarySerialization = !Enum;
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Byte;
    valuePtr->Data = new uint8;
  }
  if (bUseBinarySerialization)
  {
    s << valuePtr->GetByte();
  }
  else if (s.IsReading())
  {
    FName eValue;
    s << eValue;
    valuePtr->Enum = Enum;
    valuePtr->GetByte() = Enum->FindEnumIndex(eValue);
    if (Enum->NumEnums() < valuePtr->GetByte())
    {
      valuePtr->GetByte() = Enum->NumEnums() - 1;
    }
  }
  else
  {
    FName eValue(s.GetPackage());
    // TODO: maybe a bug during writing. Names table might not have eValue's string
    if (valuePtr->GetByte() < Enum->NumEnums() - 1)
    {
      eValue.SetString(Enum->GetEnum(valuePtr->GetByte()).String());
    }
    else
    {
      eValue.SetString(NAME_None);
    }
    s << eValue;
  }
}

FPropertyTag* UByteProperty::CreatePropertyTag(UObject* object)
{
  if (!object)
  {
    return nullptr;
  }
  FPropertyTag* tag = new FPropertyTag(object, GetObjectNameString(), NAME_ByteProperty);
  tag->ClassProperty = this;
  tag->ArrayDim = ArrayDim;
  tag->Value = new FPropertyValue(tag);
  tag->Value->Field = this;
  tag->Value->Type = FPropertyValue::VID::Byte;
  tag->Value->Data = new uint8;
  tag->Value->Enum = Enum;
  if (Enum)
  {
    tag->EnumName = FName(object->GetPackage(), Enum->GetObjectNameString());
  }
  return tag;
}

void UDelegateProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Delegate;
    valuePtr->Data = new FScriptDelegate;
  }
  s << valuePtr->GetScriptDelegate();
}

void UIntProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Int;
    valuePtr->Data = new int32;
  }
  s << valuePtr->GetInt();
}

FPropertyTag* UIntProperty::CreatePropertyTag(UObject* object)
{
  if (!object)
  {
    return nullptr;
  }
  FPropertyTag* tag = new FPropertyTag(object, GetObjectNameString(), NAME_IntProperty);
  tag->ArrayDim = ArrayDim;
  tag->ClassProperty = this;
  tag->Value = new FPropertyValue(tag);
  tag->Value->Field = this;
  tag->Value->Type = FPropertyValue::VID::Int;
  tag->Value->Data = new int32;
  return tag;
}

void UBoolProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Bool;
    valuePtr->Data = new bool;
  }
  uint8 tmp = valuePtr->GetBool();
  s << tmp;
  valuePtr->GetBool() = tmp;
}

FPropertyTag* UBoolProperty::CreatePropertyTag(UObject* object)
{
  if (!object)
  {
    return nullptr;
  }
  FPropertyTag* tag = new FPropertyTag(object, GetObjectNameString(), NAME_BoolProperty);
  tag->ClassProperty = this;
  tag->ArrayDim = ArrayDim;
  tag->Value = new FPropertyValue(tag);
  tag->Value->Field = this;
  tag->Value->Type = FPropertyValue::VID::Bool;
  tag->Value->Data = new bool;
  return tag;
}

void UFloatProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Float;
    valuePtr->Data = new float;
  }
  s << valuePtr->GetFloat();
}

FPropertyTag* UFloatProperty::CreatePropertyTag(UObject* object)
{
  if (!object)
  {
    return nullptr;
  }
  FPropertyTag* tag = new FPropertyTag(object, GetObjectNameString(), NAME_FloatProperty);
  tag->ArrayDim = ArrayDim;
  tag->ClassProperty = this;
  tag->Value = new FPropertyValue(tag);
  tag->Value->Field = this;
  tag->Value->Type = FPropertyValue::VID::Float;
  tag->Value->Data = new float;
  return tag;
}

void UNameProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Name;
    valuePtr->Data = new FName;
  }
  s << valuePtr->GetName();
}

FPropertyTag* UNameProperty::CreatePropertyTag(UObject* object)
{
  if (!object)
  {
    return nullptr;
  }
  FPropertyTag* tag = new FPropertyTag(object, GetObjectNameString(), NAME_NameProperty);
  tag->ArrayDim = ArrayDim;
  tag->ClassProperty = this;
  tag->Value = new FPropertyValue(tag);
  tag->Value->Field = this;
  tag->Value->Type = FPropertyValue::VID::Name;
  tag->Value->Data = new FName(object->GetPackage());
  return tag;
}

void UStrProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::String;
    valuePtr->Data = new FString;
  }
  s << valuePtr->GetString();
}

FPropertyTag* UStrProperty::CreatePropertyTag(UObject* object)
{
  if (!object)
  {
    return nullptr;
  }
  FPropertyTag* tag = new FPropertyTag(object, GetObjectNameString(), NAME_StrProperty);
  tag->ArrayDim = ArrayDim;
  tag->ClassProperty = this;
  tag->Value = new FPropertyValue(tag);
  tag->Value->Field = this;
  tag->Value->Type = FPropertyValue::VID::String;
  tag->Value->Data = new FString;
  return tag;
}

void UArrayProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  // TODO: this is super inefficient for RawData properties. Each byte has its own allocation. Refactor needed.
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Array;
    valuePtr->Data = new std::vector<FPropertyValue*>;
  }
  int32 elementCount = (int32)valuePtr->GetArray().size();
  s << elementCount;

  for (int32 idx = 0; idx < elementCount; ++idx)
  {
    if (s.IsReading())
    {
      valuePtr->GetArray().push_back(new FPropertyValue(valuePtr->Property));
    }
    Inner->SerializeItem(s, valuePtr->GetArray()[idx], object, defaults);
  }
}

FPropertyTag* UArrayProperty::CreatePropertyTag(UObject* object)
{
  if (!object)
  {
    return nullptr;
  }
  FPropertyTag* tag = new FPropertyTag(object, GetObjectNameString(), NAME_ArrayProperty);
  tag->ClassProperty = this;
  tag->ArrayDim = ArrayDim;
  tag->Value = new FPropertyValue(tag);
  tag->Value->Field = this;
  tag->Value->Type = FPropertyValue::VID::Array;
  tag->Value->Data = new std::vector<FPropertyValue*>;
  return tag;
}

void UStructProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  bool bUseBinarySerialization = (Struct->StructFlags & STRUCT_ImmutableWhenCooked) || (Struct->StructFlags & STRUCT_Immutable);
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Struct;
    valuePtr->Data = new std::vector<FPropertyValue*>;
    valuePtr->Struct = Struct;
  }

  if (bUseBinarySerialization)
  {
    Struct->SerializeBin(s, valuePtr, object);
  }
  else
  {
    Struct->SerializeTaggedProperties(s, object, valuePtr, Struct, defaults);
  }
}

FPropertyTag* UStructProperty::CreatePropertyTag(UObject* object)
{
  if (!object)
  {
    return nullptr;
  }
  FPropertyTag* tag = new FPropertyTag(object, GetObjectNameString(), NAME_StructProperty);
  tag->ClassProperty = this;
  tag->ArrayDim = ArrayDim;
  tag->Value = new FPropertyValue(tag);
  tag->Value->Struct = Struct;
  tag->Value->Field = this;
  tag->Value->Type = FPropertyValue::VID::Struct;
  tag->Value->Data = new std::vector<FPropertyValue*>;
  if (Struct)
  {
    tag->StructName = FName(object->GetPackage(), Struct->GetObjectNameString());
  }
  return tag;
}

void UObjectProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  UObject* tmp = nullptr;
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Object;
    valuePtr->Data = new PACKAGE_INDEX;
  }
  else
  {
    tmp = valuePtr->Property->Owner->GetPackage()->GetObject(valuePtr->GetObjectIndex());
  }
  s.SerializeObjectRef((void*&)tmp, valuePtr->GetObjectIndex());
}

FPropertyTag* UObjectProperty::CreatePropertyTag(UObject* object)
{
  if (!object)
  {
    return nullptr;
  }
  FPropertyTag* tag = new FPropertyTag(object, GetObjectNameString(), NAME_ObjectProperty);
  tag->ArrayDim = ArrayDim;
  tag->ClassProperty = this;
  tag->Value = new FPropertyValue(tag);
  tag->Value->Field = this;
  tag->Value->Type = FPropertyValue::VID::Object;
  tag->Value->Data = new PACKAGE_INDEX(INDEX_NONE);
  return tag;
}
