#include "UProperty.h"
#include "FStream.h"
#include "FPropertyTag.h"
#include "FStructs.h"

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

void UInterfaceProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  DBreak();
}

void UMapProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  DBreak();
}

void UByteProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  const bool bUseBinarySerialization = (Enum == nullptr);
  uint8 value = 0;
  if (bUseBinarySerialization)
  {
    s << value;
  }
  else if (s.IsReading())
  {
    FName EnumValueName;
    s << EnumValueName;

    value = Enum->FindEnumIndex(EnumValueName);
    if (Enum->NumEnums() < value)
    {
      value = Enum->NumEnums() - 1;
    }
  }
  else
  {
    FName EnumValueName(s.GetPackage());
    uint8 ByteValue = value;

    if (ByteValue < Enum->NumEnums() - 1)
    {
      EnumValueName = Enum->GetEnum(ByteValue);
    }
    else
    {
      EnumValueName.SetString(NAME_None);
    }
    s << EnumValueName;
  }
}

void UDelegateProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Function << SourceDelegate;
}

void UDelegateProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  FScriptDelegate delegate;
  s << delegate;
}

void UIntProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  int32 value;
  s << value;
}

void UBoolProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  uint8 b = tag->BoolVal;
  s << b;
}

void UFloatProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  float v;
  s << v;
}

void UNameProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  FName name;
  s << name;
}

void UStrProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  FString value;
  s << value;
}

void UArrayProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  int32 elementSize = 0;
  int32 elementCount = 0;
  s << elementCount;
  for (int32 idx = 0; idx < elementCount; ++idx)
  {
    Inner->SerializeItem(s, tag, object, defaults);
  }
}
void UStructProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  bool bUseBinarySerialization = (Struct->StructFlags & STRUCT_ImmutableWhenCooked) || (Struct->StructFlags & STRUCT_Immutable);
  if (bUseBinarySerialization)
  {
    Struct->SerializeBin(s, tag, object);
  }
  else
  {
    Struct->SerializeTaggedProperties(s, object, Struct, defaults);
  }
}

void UObjectProperty::SerializeItem(FStream& s, FPropertyTag* tag, UObject* object, UStruct* defaults) const
{
  UObject* obj = nullptr;
  s << obj;
}