#pragma once
#include "Core.h"
#include "FStream.h"
#include "FPropertyTag.h"

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

#define __GLUE_PROP(TName, Suffix) TName##Suffix
#define UPROP(TType, TName, TDefault) TType TName = TDefault; const char* P_##TName = #TName; FPropertyTag* __GLUE_PROP(TName, Property) = nullptr
#define UPROP_NOINIT(TType, TName) TType TName; const char* P_##TName = #TName; FPropertyTag* __GLUE_PROP(TName, Property) = nullptr
#define PROP_IS(prop, TName) (prop->Name == P_##TName)

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

#define REGISTER_FLOAT_PROP(TName) __REGISTER_PROP(TName, Float)

#define REGISTER_INT_PROP(TName) __REGISTER_PROP(TName, Int)

#define REGISTER_BOOL_PROP(TName) __REGISTER_PROP(TName, Bool)

#define REGISTER_BYTE_PROP(TName) __REGISTER_PROP(TName, Byte)

#define REGISTER_STR_PROP(TName) __REGISTER_PROP(TName, String)

#define REGISTER_OBJ_PROP(TName) __REGISTER_PROP(TName, ObjectValuePtr)

#define REGISTER_TOBJ_PROP(TName, TType) __REGISTER_TYPED_PROP(TName, TType, ObjectValuePtr)

#define REGISTER_NAME_PROP(TName) __REGISTER_PROP(TName, Name)

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

class UObject {
public:
  enum { StaticClassCastFlags = CASTCLASS_None };
  virtual uint32 GetStaticClassCastFlags() const
  {
    return StaticClassCastFlags;
  }

  static void LoadObject(UObject* object)
  {
    if (object)
    {
      object->Load();
    }
  }

  // Object factory
  static UObject* Object(FObjectExport* exp);

  // Cpp class name. Use ONLY with a class (e.g. UObject::StaticClass())
  static const char* StaticClassName()
  {
    return "Object";
  }

  UObject() = delete;
  UObject(const UObject&) = delete;
  UObject(FObjectExport* exp);

  // Get cpp class name from an instance
  virtual const char* GetStaticClassName() const
  {
    return StaticClassName();
  }

  virtual std::string GetStaticClassChain() const
  {
    return StaticClassName();
  }

  virtual ~UObject();

  // Load the object from its package
  virtual void Load();
  virtual void Load(FStream& s);

  // Serialize object by an index
  friend FStream& operator<<(FStream& s, UObject*& obj);

  virtual bool RegisterProperty(FPropertyTag* property);

  FString GetObjectPath() const;

  FString GetObjectName() const;

  FString GetClassName() const;

  uint32 GetExportFlags() const;

  uint64 GetObjectFlags() const;

  FString GetFullObjectName() const;

  inline FObjectExport* GetExportObject() const
  {
    return Export;
  }

  inline NET_INDEX GetNetIndex() const
  {
    return NetIndex;
  }

  FPackage* GetPackage() const;

  virtual bool IsComponent() const
  {
    return false;
  }

  inline bool HasAnyFlags(uint64 flags) const;

  UObject* GetOuter() const
  {
    return Outer;
  }

  std::vector<UObject*> GetInner() const
  {
    return Inner;
  }

  void MarkDirty(bool dirty = true);

  inline bool IsDirty() const
  {
    return HasAnyFlags(RF_Marked);
  }

  FILE_OFFSET GetSerialOffset() const;
  
  FILE_OFFSET GetSerialSize() const;

  FILE_OFFSET GetPropertiesSize() const;

  FILE_OFFSET GetDataSize() const;

  bool IsTemplate(uint64 templateTypes = (RF_ArchetypeObject | RF_ClassDefaultObject)) const
  {
    for (const UObject* outer = this; outer; outer = outer->GetOuter())
    {
      if (outer->HasAnyFlags((EObjectFlags)templateTypes))
      {
        return true;
      }
    }
    return false;
  }

  template<typename T>
  T* GetTypedOuter() const
  {
    T* result = NULL;
    for (UObject* nextOuter = Outer; result == NULL && nextOuter != NULL; nextOuter = nextOuter->GetOuter())
    {
      if (nextOuter->IsA(T::StaticClassName()))
      {
        result = (T*)nextOuter;
      }
    }
    return result;
  }

  bool IsA(const char* base) const;

  inline bool IsLoaded() const
  {
    return Loaded;
  }

  inline UClass* GetClass() const
  {
    return Class;
  }

  std::vector<FPropertyTag*> GetProperties() const
  {
    return Properties;
  }

  void AddProperty(FPropertyTag* property)
  {
    if (property->Name != NAME_None && Properties.size() && Properties.back()->Name == NAME_None)
    {
      Properties.insert(Properties.end() - 1, property);
    }
    else
    {
      Properties.push_back(property);
    }
  }

  void RemoveProperty(FPropertyTag* tag);
  
  // Object initialization. Wont modify package's object tree!
  void SetOuter(UObject* outer)
  {
    Outer = outer;
  }

  // Object initialization. Wont modify package's object tree!
  void AddInner(UObject* inner)
  {
    Inner.push_back(inner);
  }

  void* GetRawData();

  void SetRawData(void* data, FILE_OFFSET size);

  // Serialize the object from a stream. Should not be called outside of the Load
  virtual void Serialize(FStream& s);

private:
  virtual void SerializeScriptProperties(FStream& s);

protected:
  // Handle object initialization for derived classes
  virtual void PostLoad();

  void SerializeDefaultObject(FStream& s);

  void SerializeTrailingData(FStream& s);

protected:
  bool Loaded = false;
  FObjectExport* Export = nullptr;
  FStateFrame* StateFrame = nullptr;
  NET_INDEX NetIndex = INDEX_NONE;

  UClass* Class = nullptr;
  UObject* DefaultObject = nullptr;

  UObject* Outer = nullptr;
  std::vector<UObject*> Inner;

  std::vector<FPropertyTag*> Properties;

  FILE_OFFSET RawDataOffset = 0;
  FILE_OFFSET RawDataSize = 0;
  char* RawData = nullptr;

  void* TrailingData = nullptr;
  FILE_OFFSET TrailingDataSize = 0;
#ifdef _DEBUG
  std::string Description;
#endif
private:
  bool Loading = false;
};