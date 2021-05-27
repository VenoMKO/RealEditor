#include "FObjectResource.h"
#include "FStream.h"
#include "FPackage.h"
#include "UObject.h"
#include "UClass.h"

FStream& operator<<(FStream& s, FObjectImport& i)
{
  if (s.IsReading() && s.GetPackage())
  {
    i.Package = s.GetPackage();
  }
  s << i.ClassPackage;
  s << i.ClassName;
  s << i.OuterIndex;
  s << i.ObjectName;
  return s;
}

FStream& operator<<(FStream& s, FObjectExport& e)
{
  if (s.IsReading() && s.GetPackage())
  {
    e.Package = s.GetPackage();
  }
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

FString FObjectExport::GetClassName() const
{
  return ClassIndex ? Package->GetResourceObject(ClassIndex)->GetObjectName() : "Class";
}

FObjectImport* FObjectImport::CreateImport(FPackage* package, const FString& objectName, UClass* objectClass)
{
  if (!package || objectName.Empty())
  {
    return nullptr;
  }
  FObjectImport* result = new FObjectImport(package, objectName);
  package->GetNameIndex(objectName, true);
  package->GetNameIndex(objectClass ? objectClass->GetObjectName() : "Class", true);
  result->ClassName = FName(package, objectClass ? objectClass->GetObjectName() : "Class");
  result->ClassPackage = FName(package, objectClass ? objectClass->GetPackage()->GetPackageName() : "Core");
  return result;
}

FString FObjectImport::GetPackageName() const
{
  PACKAGE_INDEX outerIndex = OuterIndex;
  FObjectResource* outer = nullptr;
  while (outerIndex)
  {
    if (outerIndex < 0)
    {
      outer = Package->GetImportObject(outerIndex);
      outerIndex = outer->OuterIndex;
    }
    else
    {
      // Imports outer object is Export. Not sure what is the correct behavior
      outer = Package->GetExportObject(outerIndex);
      outerIndex = 0;
    }
  }
  return outer ? outer->ObjectName.String(true) : FString();
}

FString FObjectImport::GetObjectPath() const
{
  FObjectResource* outer = GetOuter();
  FString path;
  while (outer)
  {
    path = outer->GetObjectName() + "." + path;
    outer = outer->GetOuter();
  }
  path += GetObjectName();
  return path;
}

FObjectResource* FObjectResource::GetOuter() const
{
  if (OuterIndex)
  {
    return Package->GetResourceObject(OuterIndex);
  }
  return nullptr;
}

FString FObjectResource::GetObjectPath() const
{
  if (FObjectResource* outer = GetOuter())
  {
    return outer->GetObjectPath() + "." + GetObjectName();
  }
  return Package->GetPackageName() + "." + GetObjectName();
}

VObjectExport::VObjectExport(FPackage* pkg, const char* objName, const char* className)
  : FObjectExport(pkg)
{
  VObjectName = objName;
  VObjectClassName = className;
  ObjectIndex = VEXP_INDEX;
}

FString VObjectExport::GetObjectPath() const
{
  return Package->GetPackageName() + "." + GetObjectName();
}

VObjectExport::~VObjectExport()
{
  if (VObject)
  {
    delete VObject;
  }
}
