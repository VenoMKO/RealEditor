#pragma once
#include "Core.h"
#include "UObject.h"
#include "FName.h"
#include "FPropertyTag.h"

#include <mutex>

struct FPropertyTag;

#define DECL_CLASS_INERIT(Class)\
  enum { StaticClassCastFlags = CASTCLASS_##Class};\
  uint32 GetStaticClassCastFlags() const override\
  { return StaticClassCastFlags | Super::GetStaticClassCastFlags(); }\


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

protected:
  void Serialize(FStream& s);

protected:
  int32 Pos = 0;
  int32 Top = 0;
  FString Text;
};

class UField : public UObject {
public:
  DECL_UOBJ(UField, UObject);
  DECL_CLASS_INERIT(UField);

  inline UField* GetSuperfield() const
  {
    return Superfield;
  }

  inline void SetSuperfield(UField* field)
  {
    Superfield = field;
  }

  inline UField* GetNext() const
  {
    return Next;
  }

  inline void SetNext(UField* field)
  {
    Next = field;
  }

protected:
  void Serialize(FStream& s) override;

protected:
  UField* Superfield = nullptr;
  UField* Next = nullptr;
};

class UEnum : public UField {
public:
  DECL_UOBJ(UEnum, UField);
  DECL_CLASS_INERIT(UEnum);

  int32 FindEnumIndex(FName& inName) const
  {
    for (int32 i = 0; i < Names.size(); ++i)
    {
      if (Names[i] == inName)
      {
        return i;
      }
    }
    return INT32_MAX;
  }

  int32 NumEnums() const
  {
    return (int32)Names.size();
  }

  FName GetEnum(int32 index) const
  {
    return Names.size() > index ? Names[index] : Names.back();
  }

protected:
  void Serialize(FStream& s) override;

protected:
  std::vector<FName> Names; // Variables
};

class UStruct : public UField {
public:
  DECL_UOBJ(UStruct, UField);
  DECL_CLASS_INERIT(UStruct);

  ~UStruct();

  void SerializeTaggedProperties(FStream& s, UObject* object, FPropertyValue* value, UStruct* defaultStruct, void* defaults, int32 defaultsCount = 0) const;
  void SerializeBin(FStream& s, FPropertyValue* value, UObject* object) const;
  void SerializeBinEx(FStream& s, FPropertyValue* value, UObject* object, UStruct* defaultStruct, void* defaults, int32 defaultsCount) const;
  void SerializeBinProperty(UProperty* property, FPropertyValue* value, FStream& s, UObject* object) const;

  virtual UStruct* GetInheritanceSuper() const
  {
    return GetSuperStruct();
  }

  inline bool IsChildOf(const UStruct* SomeBase) const
  {
    for (const UStruct* Struct = this; Struct; Struct = Struct->GetSuperStruct())
    {
      if (Struct == SomeBase)
      {
        return true;
      }
    }
    return false;
  }

  inline UStruct* GetSuperStruct() const
  {
    return SuperStruct;
  }

  inline void SetSuperStruct(UStruct* field)
  {
    SuperStruct = field;
  }

  inline UField* GetChildren() const
  {
    return Children;
  }

  inline void SetChildren(UField* field)
  {
    Children = field;
  }

  virtual void Link();

protected:
  void Serialize(FStream& s) override;

protected:
  UStruct* SuperStruct = nullptr;
  UTextBuffer* CppText = nullptr;
  UTextBuffer* ScriptText = nullptr;
  UField* Children = nullptr;
  int32 Line = 0;
  int32 TextPos = 0;
  uint32 ScriptDataSize = 0;
  uint32 ScriptStorageSize = 0;
  void* ScriptData = nullptr;
  void* ScriptStorage = nullptr;
  std::mutex LinkMutex;
  UProperty* PropertyLink = nullptr;
};

class UState : public UStruct {
public:
  DECL_UOBJ(UState, UStruct);
  DECL_CLASS_INERIT(UState);

  UState* GetSuperState() const
  {
    DBreakIf(SuperStruct && !SuperStruct->IsA(UState::StaticClassName()));
    return (UState*)SuperStruct;
  }

  UStruct* GetInheritanceSuper() const
  {
    return GetSuperState();
  }

protected:
  void Serialize(FStream& s) override;

protected:
  int32 ProbeMask = 0;
  int32 StateFlags = 0;
  int16 LabelTableOffset = 0;
  std::map<FName, UObject*> FuncMap;
};

class UClass : public UState {
public:
  DECL_UOBJ(UClass, UState);
  DECL_CLASS_INERIT(UClass);

  static void CreateBuiltInClasses(FPackage* package);

  UClass(FObjectExport* exp)
    : UState(exp)
  {}

  UObject* GetClassDefaultObject()
  {
    if (ClassDefaultObject)
    {
      ClassDefaultObject->Load();
    }
    return ClassDefaultObject;
  }

  UClass* GetSuperClass() const
  {
    return (UClass*)SuperStruct;
  }

  UStruct* GetInheritanceSuper() const
  {
    return GetSuperClass();
  }

  inline bool HasAnyCastFlag(EClassCastFlag FlagToCheck) const
  {
    return (ClassCastFlags & FlagToCheck) != 0;
  }

  inline bool HasAllCastFlags(EClassCastFlag FlagsToCheck) const
  {
    return (ClassCastFlags & FlagsToCheck) == FlagsToCheck;
  }

protected:
  void Serialize(FStream& s) override;

protected:
  uint32 ClassFlags = 0;
  int32 ClassUnique = 0;
  UClass* ClassWithin = nullptr;
  FName ClassConfigName;
  bool bForceScriptOrder = false;
  std::map<FName, UObject*> ComponentNameToDefaultObjectMap;
  std::vector<FImplementedInterface> Interfaces;
  std::vector<FName> HideCategories;
  std::vector<FName> AutoExpandCategories;
  std::vector<FName> AutoCollapseCategories;
  std::vector<FName> DontSortCategories;
  std::vector<FName> ClassGroupNames;
  FString ClassHeaderFilename;
  FName DLLBindName;
  UObject* ClassDefaultObject = nullptr;

  uint32 ClassCastFlags = CASTCLASS_None;
};

class UScriptStruct : public UStruct {
public:
  DECL_UOBJ(UScriptStruct, UStruct);
  DECL_CLASS_INERIT(UScriptStruct);

protected:
  void Serialize(FStream& s) override;

public:
  uint32 StructFlags = 0;
  uint8* StructDefaults = nullptr;
  size_t StructDefaultsSize = 0;
};

class UFunction : public UStruct
{
public:
  DECL_UOBJ(UFunction, UStruct);

protected:
  void Serialize(FStream& s) override;

public:
  uint32 FunctionFlags = 0;
  uint16 iNative = 0;
  uint16 RepOffset = 0;
  FName	FriendlyName;
  uint8	OperPrecedence = 0;
};


class UConst : public UField
{
public:
  DECL_UOBJ(UConst, UField);
  DECL_CLASS_INERIT(UConst);

protected:
  void Serialize(FStream& s) override;

public:
  FString Value;
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
  uint64 ProbeMask64 = 0;
  uint32 ProbeMask32 = 0;
  uint16 LatentAction = 0;
  std::vector<FPushedState> StateStack;
  int32 Offset = 0;
};

template <class T>
class TFieldIterator
{
  struct PrivateBooleanHelper { int32 Value = 0; };

protected:
  /** The object being searched for the specified field */
  const UStruct* Struct = nullptr;
  /** The current location in the list of fields being iterated */
  UField* Field = nullptr;
  /** Whether to include the super class or not */
  const bool bShouldIncludeSuper = true;

public:
  TFieldIterator(const UStruct* strct, const bool includeSuper = true)
    : Struct(strct)
    , Field(strct ? strct->GetChildren() : nullptr)
    , bShouldIncludeSuper(includeSuper)
  {
    IterateToNext();
  }
  /** conversion to "bool" returning TRUE if the iterator is valid. */
  typedef bool PrivateBooleanType;
  inline operator PrivateBooleanType() const
  {
    return Field != nullptr ? &PrivateBooleanHelper::Value : nullptr;
  }
  inline bool operator !() const
  {
    return !operator PrivateBooleanType();
  }

  inline void operator++()
  {
    Field = Field->GetNext();
    IterateToNext();
  }

  inline T* operator*()
  {
    return (T*)Field;
  }

  inline T* operator->()
  {
    return (T*)Field;
  }

  inline const UStruct* GetStruct()
  {
    return Struct;
  }

protected:
  inline void IterateToNext()
  {
    UField* currentField = Field;
    const UStruct* currentStruct = Struct;
    while (currentStruct)
    {
      while (currentField)
      {
        if (currentField->GetStaticClassCastFlags() & T::StaticClassCastFlags)
        {
          Struct = currentStruct;
          Field = currentField;
          return;
        }
        currentField = currentField->GetNext();
      }

      if (bShouldIncludeSuper)
      {
        currentStruct = currentStruct->GetInheritanceSuper();
        if (currentStruct)
        {
          currentField = currentStruct->GetChildren();
        }
      }
      else
      {
        currentStruct = nullptr;
      }
    }

    Struct = currentStruct;
    Field = currentField;
  }
};