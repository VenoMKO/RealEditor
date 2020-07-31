#pragma once
#include "Core.h"
#include "FStream.h"

// Common UObject subclass declarations
#define DECL_UOBJ(TClass, TSuper)\
public:\
  inline friend FStream& operator<<(FStream& s, TClass*& obj) { return s << *(UObject**)&obj; }\
  typedef TClass ThisClass;\
  typedef TSuper Super;\
  static const char* StaticClassName() { return ((char*)#TClass) + 1; }\
  const char* GetStaticClassName() const override { return ThisClass::StaticClassName(); }\
  using TSuper::TSuper

class UObject {
public:
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

  virtual ~UObject();
  // Load the object from its package
  virtual void Load();
  // Serialize object by an index
  friend FStream& operator<<(FStream& s, UObject*& obj);

  std::string GetObjectPath() const;

  std::string GetObjectName() const;

  std::string GetClassName() const;

  uint32 GetExportFlags() const;

  uint64 GetObjectFlags() const;

  inline FObjectExport* GetExportObject()
  {
    return Export;
  }

  inline FObjectImport* GetImportObject()
  {
    return Import;
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

  UObject* GetOuter() const;

  bool IsTemplate(uint64 templateTypes = (RF_ArchetypeObject | RF_ClassDefaultObject)) const
  {
    for (const UObject* outer = this; outer; outer = outer->GetOuter())
    {
      if (outer->GetObjectFlags() & templateTypes)
      {
        return true;
      }
    }
    return false;
  }

private:
  virtual void SerializeScriptProperties(FStream& s) const;

protected:
  // Serialize the object from a stream. Should not be called outside of the Load
  virtual void Serialize(FStream& s);
  // Handle object initializtion for derrived classes
  virtual void PostLoad();

protected:
  bool Loaded = false;
  FObjectExport* Export = nullptr;
  FObjectImport* Import = nullptr;
  FStateFrame* StateFrame = nullptr;
  NET_INDEX NetIndex = INDEX_NONE;

  UClass* Class = nullptr;
  UObject* DefaultObject = nullptr;

  UObject* Outer = nullptr;

  FILE_OFFSET RawDataOffset = 0;
  FILE_OFFSET RawDataSize = 0;
  char* RawData = nullptr;
#ifdef _DEBUG
  std::string Description;
#endif
private:
  bool Loading = false;
};