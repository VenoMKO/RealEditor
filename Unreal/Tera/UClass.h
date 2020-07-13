#pragma once
#include "Core.h"
#include "UObject.h"

#include "FName.h"

class FImplementedInterface {
public:
  FImplementedInterface()
  {}

  FImplementedInterface(const FImplementedInterface& i)
  {
    ObjectClass = i.ObjectClass;
    PointerProperty = i.PointerProperty;
  }

  friend FStream& operator<<(FStream& s, FImplementedInterface& i);

protected:
  UObject* ObjectClass = nullptr;
  UObject* PointerProperty = nullptr;
};


class UTextBuffer : public UObject {
public:
  DECL_UOBJ(UTextBuffer, UObject);

  void Serialize(FStream& s);

protected:
  int32 Pos = 0;
  int32 Top = 0;
  std::string Text;
};

class UField : public UObject {
public:
  DECL_UOBJ(UField, UObject);

  inline UField* GetSuperfield()
  {
    return Superfield;
  }

  inline UField* GetNext()
  {
    return Next;
  }

  void Serialize(FStream& s) override;

protected:
  UField* Superfield = nullptr;
  UField* Next = nullptr;
};

class UEnum : public UField {
public:
  DECL_UOBJ(UEnum, UField);

  void Serialize(FStream& s) override;

protected:
  std::vector<FName> Names; // Variables
};

class UStruct : public UField {
public:
  DECL_UOBJ(UStruct, UField);
  ~UStruct();

  void Serialize(FStream& s);

protected:
  UTextBuffer* CppText = nullptr;
  UTextBuffer* ScriptText = nullptr;
  UField* Children = nullptr;
  int32 Line = 0;
  int32 TextPos = 0;
  uint32 ScriptDataSize = 0;
  void* ScriptData = nullptr;
};

class UState : public UStruct {
public:
  DECL_UOBJ(UState, UStruct);

  void Serialize(FStream& s);

protected:
  int32 Unk;
  int32 ProbeMask;
  int32 StateFlags;
  int16 LabelTableOffset;
  uint64 IgnoreMask;
  std::map<FName, UObject*> FuncMap;
};

class UClass : public UState {
public:
  DECL_UOBJ(UClass, UState);

  UClass(FObjectExport* exp)
    : UState(exp)
  {}

  void Serialize(FStream& s) override;

protected:
  uint32 ClassFlags = 0;
  int32 ClassUnique = 0;
  UClass* ClassWithin = nullptr;
  FName ClassConfigName;
  std::map<FName, UObject*> ComponentNameToDefaultObjectMap;
  std::vector<FImplementedInterface> Interfaces;
  std::vector<FName> HideCategories;
  std::vector<FName> AutoExpandCategories;
  UObject* ClassDefaultObject = nullptr;
};

class FPushedState {
public:

  friend FStream& operator<<(FStream& s, FPushedState& f);

protected:
  UState* State = nullptr;
  UStruct* Node = nullptr;
};

class FStateFrame {
public:

  friend FStream& operator<<(FStream& s, FStateFrame& f);

protected:
  UState* Node = nullptr;
  UState* StateNode = nullptr;
  uint64 ProbeMask = 0;
  uint16 LatentAction = 0;
  std::vector<FPushedState> StateStack;
  int32 Offset = 0;
};