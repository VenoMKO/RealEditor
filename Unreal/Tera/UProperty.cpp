#include "UProperty.h"
#include "FStream.h"

#define CPF_Net 0x0000000000000020

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

void UObjectProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
  s << *(UObject**)value;
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

void UInterfaceProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
}

void UArrayProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Inner;
}

void UArrayProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
}

void UMapProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Key;
  s << Value;
}

void UMapProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
}

void UStructProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Struct;
}

void UStructProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
  //bool bUseBinarySerialization = (Struct->StructFlags & STRUCT_ImmutableWhenCooked) != 0
  Struct->SerializeTaggedProperties(s, (UObject*)value, Struct, defaults);
}

void UByteProperty::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Enum;
}

void UByteProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
  const bool bUseBinarySerialization = (Enum == NULL);
  if (bUseBinarySerialization)
  {
    s << *(uint8*)value;
  }
  else if (s.IsReading())
  {
    FName EnumValueName;
    s << EnumValueName;

    *(uint8*)value = Enum->FindEnumIndex(EnumValueName);
    if (Enum->NumEnums() < *(uint8*)value)
    {
      *(uint8*)value = Enum->NumEnums() - 1;
    }
  }
  else
  {
    FName EnumValueName(s.GetPackage());
    uint8 ByteValue = *(uint8*)value;

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

void UDelegateProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
}

void UIntProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
  s << *(int32*)value;
}

void UBoolProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
  uint8 b = (*(BITFIELD*)value & BitMask) ? 1 : 0;
  s << b;
  if (b)
  {
    *(BITFIELD*)value |= BitMask;
  }
  else
  {
    *(BITFIELD*)value &= ~BitMask;
  }
}

void UFloatProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
  s << *(float*)value;
}

void UNameProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
  s << *(FName*)value;
}

void UStrProperty::SerializeItem(FStream& s, void* value, int32 maxReadBytes, void* defaults) const
{
  s << *(FString*)value;
}
