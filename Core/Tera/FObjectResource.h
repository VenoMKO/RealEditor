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

  virtual ~FObjectResource()
  {}

  virtual FString GetObjectName() const
  {
    FString name;
    ObjectName.GetString(name);
    return name;
  }

  inline FString GetFullObjectName() const
  {
    return GetClassName() + " " + GetObjectPath();
  }

  virtual FString GetClassName() const = 0;

  FObjectResource* GetOuter() const;

  virtual FString GetObjectPath() const;

  
  PACKAGE_INDEX OuterIndex = INDEX_NONE;
  PACKAGE_INDEX ObjectIndex = 0;
  FPackage* Package = nullptr;
#ifdef _DEBUG
  FString Path;
#endif
protected:
  FName ObjectName;
};

class FObjectImport : public FObjectResource {
public:
  using FObjectResource::FObjectResource;

  static FObjectImport* CreateImport(FPackage* package, const FString& objectName, UClass* objectClass);

  FObjectImport(FPackage* p)
    : FObjectResource(p)
    , ClassName(p)
  {}

  FString GetClassName() const override
  {
    FString name;
    ClassName.GetString(name);
    return name;
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
  FObjectExport(FPackage* p)
    : FObjectResource(p)
  {}

  friend FStream& operator<<(FStream& s, FObjectExport& e);

  FString GetClassName() const override;

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

  inline FString GetObjectName() const override
  {
    return VObjectName;
  }

  inline FString GetClassName() const override
  {
    return VObjectClassName;
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

  ~VObjectExport() override;

private:
  FString VObjectName;
  FString VObjectClassName;
  UObject* VObject = nullptr;
};