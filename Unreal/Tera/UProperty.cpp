#include "UProperty.h"
#include "FStream.h"

#define CPF_Net 0x0000000000000020

const char* NAME_PropNone = "None";
const char* NAME_PropBool = "BoolProperty";
const char* NAME_PropByte = "ByteProperty";
const char* NAME_PropInt = "IntProperty";
const char* NAME_PropFloat = "FloatProperty";
const char* NAME_PropObj = "ObjectProperty";
const char* NAME_PropName = "NameProperty";
const char* NAME_PropString = "StrProperty";
const char* NAME_PropStruct = "StructProperty";
const char* NAME_PropArray = "ArrayProperty";
const char* NAME_PropMap = "MapProperty";
const char* NAME_PropInterface = "InterfaceProperty";
const char* NAME_PropDelegate = "DelegateProperty";

const char* NAME_PropStructVector = "Vector";
const char* NAME_PropStructVector4 = "Vector4";
const char* NAME_PropStructColor = "Color";
const char* NAME_PropStructLinearColor = "LinearColor";
const char* NAME_PropStructGuid = "Guid";
const char* NAME_PropStructRotator = "Rotator";
const char* NAME_PropStructMatrix = "Matrix";

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