#pragma once
#include "Core.h"
#include "FStructs.h"
#include "FName.h"

#define VEXP_INDEX 0x80000

class FObjectResource {
public:
  FObjectResource(FPackage* package)
    : Package(package)
  {}

  FObjectResource(FPackage* package, const FString& name)
    : Package(package)
    , ObjectName(package, name)
  {}

  virtual ~FObjectResource() = default;

  virtual FName GetObjectName() const
  {
    return ObjectName;
  }

  virtual FString GetClassNameString() const
  {
    return GetClassName().String();
  }

  virtual FString GetObjectNameString() const
  {
    return ObjectName.String();
  }

  inline FString GetFullObjectName() const
  {
    return GetClassNameString() + " " + GetObjectPath();
  }

  virtual FName GetClassName() const = 0;

  FObjectResource* GetOuter() const;

  virtual FString GetObjectPath() const;

  
  PACKAGE_INDEX OuterIndex = 0;
  PACKAGE_INDEX ObjectIndex = 0;
  FPackage* Package = nullptr;
  FName ObjectName;
#ifdef _DEBUG
  FString Path;
#endif
};

class FObjectImport : public FObjectResource {
public:
  using FObjectResource::FObjectResource;

  static FObjectImport* CreateImport(FPackage* package, const FString& objectName, UClass* objectClass);

  FObjectImport(FPackage* p)
    : FObjectResource(p)
    , ClassName(p)
  {}

  FName GetClassName() const override
  {
    return ClassName;
  }

  FString GetClassNameString() const override
  {
    return ClassName.String();
  }

  FString GetPackageName() const;

  FString GetObjectPath() const override;

  friend FStream& operator<<(FStream& s, FObjectImport& i);

  FName ClassPackage;
  FName ClassName;

  std::vector<FObjectImport*> Inner;
};

class FObjectExport : public FObjectResource {
public:
  using FObjectResource::FObjectResource;

  friend FStream& operator<<(FStream& s, FObjectExport& e);

  FName GetClassName() const override;

  PACKAGE_INDEX ClassIndex = 0;
  PACKAGE_INDEX SuperIndex = 0;
  PACKAGE_INDEX	ArchetypeIndex = 0;
  uint64 ObjectFlags = 0;
  FILE_OFFSET SerialSize = 0;
  FILE_OFFSET SerialOffset = 0;
  uint32 ExportFlags = EF_None;
  std::vector<int32> GenerationNetObjectCount;
  FGuid PackageGuid;
  uint32 PackageFlags = 0;

  FObjectExport* Outer = nullptr;
  std::vector<FObjectExport*> Inner;

#ifdef _DEBUG
  std::string ClassNameValue;
#endif
};

class VObjectExport : public FObjectExport {
public:
  VObjectExport(FPackage* package, const char* objName, const char* className);

  inline FName GetClassName() const override
  {
    return VObjectClassName;
  }

  inline FString GetClassNameString() const override
  {
    return VObjectClassName.String();
  }

  FString GetObjectPath() const override;

  inline UObject* GetObject() const
  {
    return VObject;
  }

  inline void SetObject(UObject* obj)
  {
    DBreakIf(VObject);
    VObject = obj;
  }

  ~VObjectExport();

private:
  FString VObjectName;
  VName   VObjectClassName;
  UObject* VObject = nullptr;
};