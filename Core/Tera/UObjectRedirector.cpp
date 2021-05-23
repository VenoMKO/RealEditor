#include "UObjectRedirector.h"
#include "FPackage.h"
#include "FObjectResource.h"
#include "FName.h"

void UObjectRedirector::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, Object);
  if (s.IsReading() && !Object && ObjectRefIndex != INDEX_NONE && ObjectRefIndex < 0)
  {
    if (FObjectImport* imp = GetPackage()->GetImportObject(ObjectRefIndex))
    {
      FObjectImport* package = (FObjectImport*)imp->GetOuter();
      while (package->GetOuter())
      {
        package = (FObjectImport*)package->GetOuter();
      }
      if (!package)
      {
        return;
      }
      std::vector<FString> parts = package->GetObjectName().Split('_');
      parts.pop_back();
      if (parts.empty())
      {
        return;
      }
      FString partName;
      for (const FString& part : parts)
      {
        partName += part + "_";
      }
      auto map = FPackage::GetCompositePackageMap();
      FString result;
      for (const auto& p : map)
      {
        if (p.first.StartWith(partName))
        {
          if (std::shared_ptr<FPackage> package = FPackage::GetPackageNamed(p.first))
          {
            package->Load();
            if (AltObject = package->GetObject(INDEX_NONE, imp->GetObjectName(), imp->GetClassName()))
            {
              GetPackage()->RetainPackage(package);
            }
            FPackage::UnloadPackage(package);
            if (AltObject)
            {
              break;
            }
          }
        }
      }
    }
  }
}