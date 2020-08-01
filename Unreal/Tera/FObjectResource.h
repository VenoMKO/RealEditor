#pragma once
#include "Core.h"
#include "FStructs.h"
#include "FName.h"

class FObjectResource {
public:
  FObjectResource(FPackage* package)
    : Package(package)
  {}

  virtual ~FObjectResource()
  {}

  inline std::string GetObjectName() const
  {
    std::string name;
    ObjectName.GetString(name);
    return name;
  }

  inline std::string GetFullObjectName() const
  {
    return GetClassName() + " " + GetObjectPath();
  }

  virtual std::string GetClassName() const = 0;

  FObjectResource* GetOuter() const;

  std::string GetObjectPath() const;

  FName ObjectName;
  PACKAGE_INDEX OuterIndex = INDEX_NONE;
  PACKAGE_INDEX ObjectIndex = 0;

  UObject* Object = nullptr;
  FPackage* Package = nullptr;
#ifdef _DEBUG
  std::string Path;
#endif
};

class FObjectImport : public FObjectResource {
public:
  FObjectImport(FPackage* p)
    : FObjectResource(p)
  {}

  std::string GetClassName() const override
  {
    std::string name;
    ClassName.GetString(name);
    return name;
  }

  UObject* GetObject();
  std::string GetPackageName() const;

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

  std::string GetClassName() const override;
  UObject* GetObject();

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

  std::vector<FObjectExport*> Inner;

#ifdef _DEBUG
  std::string ClassNameValue;
#endif
};