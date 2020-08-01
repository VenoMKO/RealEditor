#include "UClass.h"
#include "FStream.h"

#define FUNC_Net 0x00000040

void UTextBuffer::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Pos << Top;
  s << Text;
}

void UField::Serialize(FStream& s)
{
  Super::Serialize(s);
  if (s.GetFV() < VER_TERA_MODERN)
  {
    s << Superfield;
  }
  s << Next;
}

UStruct::~UStruct()
{
  if (ScriptData)
  {
    free(ScriptData);
  }
  if (ScriptStorage)
  {
    free(ScriptStorage);
  }
}

void UStruct::Serialize(FStream& s)
{
  Super::Serialize(s);
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << SuperStruct;
  }
  s << ScriptText;
  s << Children;
  s << CppText;
  s << Line << TextPos;
  s << ScriptDataSize;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << ScriptStorageSize;
  }
  if (s.IsReading())
  {
    if (ScriptStorageSize)
    {
      ScriptStorage = malloc(ScriptStorageSize);
    }
    else if (ScriptDataSize)
    {
      ScriptData = malloc(ScriptDataSize);
    }
  }
  s.SerializeBytes(ScriptStorage, ScriptStorageSize);
  s.SerializeBytes(ScriptData, ScriptDataSize);
}

void UState::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ProbeMask;
  s << LabelTableOffset;
  s << StateFlags;
  s << FuncMap;
}

void UStruct::SerializeTaggedProperties(FStream& s, UObject* data, UStruct* defaultStruct, void* defaults, int32 defaultsCount)
{
  UClass* defaultsClass = (UClass*)defaultStruct;
}

void UClass::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ClassFlags;
  s << ClassWithin;
  s << ClassConfigName;
  s << ComponentNameToDefaultObjectMap;
  s << Interfaces;
  s << DontSortCategories;
  s << HideCategories;
  s << AutoExpandCategories;
  s << AutoCollapseCategories;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << bForceScriptOrder;
    s << ClassGroupNames;
  }
  s << ClassHeaderFilename;
  s << DLLBindName;
  s << ClassDefaultObject;
}

FStream& operator<<(FStream& s, FImplementedInterface& i)
{
  s << i.ObjectClass;
  s << i.PointerProperty;
  return s;
}

FStream& operator<<(FStream& s, FPushedState& f)
{
  s << f.State;
  s << f.Node;
  return s;
}

FStream& operator<<(FStream& s, FStateFrame& f)
{
  s << f.Node;
  s << f.StateNode;
  s << f.ProbeMask;
  s << f.LatentAction;
  s << f.StateStack;
  if (f.Node)
  {
    s << f.Offset;
  }
  return s;
}

void UEnum::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Names;
}

void UScriptStruct::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << StructFlags;
}

void UFunction::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << iNative;
  s << OperPrecedence;
  s << FunctionFlags;
  if (FunctionFlags & FUNC_Net)
  {
    s << RepOffset;
  }
  s << FriendlyName;
}

void UConst::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Value;
}
