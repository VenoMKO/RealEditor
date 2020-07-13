#include "FObjectResource.h"
#include "FStream.h"
#include "FPackage.h"

FStream& operator<<(FStream& s, FObjectImport& i)
{
  SET_PACKAGE(s, i);
  s << i.ClassPackage;
  s << i.ClassName;
  s << i.OuterIndex;
  s << i.ObjectName;
  return s;
}

FStream& operator<<(FStream& s, FObjectExport& e)
{
  SET_PACKAGE(s, e);
  s << e.ClassIndex;
  s << e.SuperIndex;
  s << e.OuterIndex;
  s << e.ObjectName;
  s << e.ArchetypeIndex;
  s << e.ObjectFlags;

  s << e.SerialSize;
  s << e.SerialOffset;

  s << e.ExportFlags;

  s << e.GenerationNetObjectCount;

  s << e.PackageGuid;
  s << e.PackageFlags;
  return s;
}

std::string FObjectExport::GetClassName() const
{
  return ClassIndex ? Package->GetResourceObject(ClassIndex)->GetObjectName() : "Class";
}

UObject* FObjectExport::GetObject()
{
  if (!Object)
  {
    Object = Package->GetObject(this);
  }
  return Object;
}

UObject* FObjectImport::GetObject()
{
  if (!Object)
  {
    Object = Package->GetObject(this);
  }
  return Object;
}

std::string FObjectImport::GetPackageName() const
{
  PACKAGE_INDEX outerIndex = OuterIndex;
  FObjectImport* outer = nullptr;
  while (outerIndex)
  {
    outer = Package->GetImportObject(outerIndex);
    outerIndex = outer->OuterIndex;
  }
  return outer ? outer->GetObjectName() : nullptr;
}
