#pragma once
#include "Core.h"

// Common UObject subclass declarations
#define DECL_UOBJ(TClass, TSuper)\
  inline friend FStream& operator<<(FStream& s, TClass*& obj) { return s << *(UObject**)&obj; }\
  typedef TSuper Super;\
  using TSuper::TSuper

class UObject {
public:
  // Object factory
  static UObject* Object(FObjectExport* exp);

  // UObjects should exist inside of a package thus have FObjectExport
  UObject() = delete;
  UObject(FObjectExport* exp);

  virtual ~UObject();

  // Load the object from a package
  virtual void Load();

  // Serialize the object from a stream. Should not be called outside of the Load
  virtual void Serialize(FStream& s);

  // Serialize object by an index
  friend FStream& operator<<(FStream& s, UObject*& obj);

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

protected:
  bool Loaded = false;
  FObjectExport* Export = nullptr;
  FObjectImport* Import = nullptr;
  FStateFrame* StateFrame = nullptr;
  NET_INDEX NetIndex = NET_INDEX_NONE;

  UClass* Class = nullptr;
#ifdef _DEBUG
  std::string Description;
#endif
};