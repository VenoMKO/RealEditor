#include "UProperty.h"
#include "FStream.h"
#include "FPropertyTag.h"
#include "FStructs.h"
#include "FString.h"

#define CPF_Net 0x0000000000000020
#define STRUCT_Immutable 0x00000020
#define STRUCT_ImmutableWhenCooked 0x00000080

void UProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ArrayDim;
  s << PropertyFlags;
  s << Category;
  s << ArraySizeEnum;
  if (PropertyFlags & CPF_Net)
  {
    s << RepOffset;
  }
}

void UObjectProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << PropertyClass;
}

void UClassProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << MetaClass;
}

void UInterfaceProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << InterfaceClass;
}

void UArrayProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Inner;
}

void UMapProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Key;
  s << Value;
}

void UStructProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Struct;
}

void UByteProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Enum;
}

void UDelegateProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Function << SourceDelegate;
}

void UInterfaceProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  DBreak();
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
      eValue = Enum->GetEnum(valuePtr->GetByte());
    }
    else
    {
      eValue.SetString(NAME_None);
    }
    s << eValue;
  }
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
    valuePtr->Data = new int;
  }
  s << valuePtr->GetInt();
}

void UBoolProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Bool;
    valuePtr->Data = new bool;
  }
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    uint8 tmp = valuePtr->GetBool();
    s << tmp;
    valuePtr->GetBool() = tmp;
  }
  else
  {
    s << valuePtr->GetBool();
  }
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

void UNameProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Name;
    valuePtr->Data = new FName;
  }
  s << valuePtr->GetName();
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

void UObjectProperty::SerializeItem(FStream& s, FPropertyValue* valuePtr, UObject* object, UStruct* defaults) const
{
  if (s.IsReading())
  {
    valuePtr->Type = FPropertyValue::VID::Object;
    valuePtr->Data = new PACKAGE_INDEX;
  }
  s << valuePtr->GetObjectIndex();
}