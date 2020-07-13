#include "UClass.h"
#include "FStream.h"

void UTextBuffer::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Pos << Top;
  s << Text;
}

void UField::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Superfield;
  s << Next;
}

UStruct::~UStruct()
{
  if (ScriptData)
  {
    free(ScriptData);
  }
}

void UStruct::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ScriptText;
  s << Children;
  s << CppText;
  s << Line << TextPos;
  s << ScriptDataSize;
  if (s.IsReading() && ScriptDataSize)
  {
    ScriptData = malloc(ScriptDataSize);
  }
  s.SerializeBytes(ScriptData, ScriptDataSize);
}

void UState::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << Unk;
  s << ProbeMask;
  s << IgnoreMask;
  s << LabelTableOffset;
  s << StateFlags;
  s << FuncMap;
}

void UClass::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ClassFlags;
  s << ClassWithin;
  s << ClassConfigName;
  s << HideCategories;
  s << ComponentNameToDefaultObjectMap;
  s << Interfaces;
  s << AutoExpandCategories;
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
