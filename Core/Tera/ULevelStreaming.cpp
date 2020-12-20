#include "ULevelStreaming.h"
#include "FPackage.h"
#include "Cast.h"

#include <Utils/ALog.h>

bool ULevelStreaming::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, PackageName))
  {
    PackageNameProperty = property;
    PackageName = property->Value->GetName().String();
    return true;
  }
  return false;
}

void ULevelStreaming::PostLoad()
{
  if (PackageName.Size() && PackageName != NAME_None)
  {
    if (LoadLevelPackage(PackageName))
    {
      // OK
      return;
    }
    auto pos = PackageName.FindLastOf('_');
    if (pos != std::string::npos)
    {
      if (US1LevelStreamingDistance* s1this = Cast<US1LevelStreamingDistance>(this))
      {
        if (s1this->ZoneNumberXProperty && s1this->ZoneNumberYProperty)
        {
          LogI("Retring with a different name...");
          if (LoadLevelPackage(PackageName.Substr(0, pos + 1) + std::to_string(s1this->ZoneNumberX) + std::to_string(s1this->ZoneNumberY)))
          {
            // OK
            return;
          }
        }
      }
      // Substr 1 from the Y coordinate of the package zone
      int32 address = -1;
      try
      {
        address += std::stoi(PackageName.Substr(pos + 1).WString());
      }
      catch (...)
      {
      }

      if (address != -1)
      {
        LogI("Retring with a different name...");
        if (LoadLevelPackage(PackageName.Substr(0, pos + 1) + std::to_string(address)))
        {
          // OK
          return;
        }
      }
    }
  }
  // Failed
}

bool ULevelStreaming::LoadLevelPackage(const FString& packageName)
{
  try
  {
    if (std::shared_ptr<FPackage> package = FPackage::GetPackageNamed(packageName.String()))
    {
      package->Load();
      Level = Cast<ULevel>(package->GetObject(INDEX_NONE, "PersistentLevel", ULevel::StaticClassName()));
      GetPackage()->RetainPackage(package);
      FPackage::UnloadPackage(package);
      return true;
    }
  }
  catch (const std::exception& e)
  {
    LogW("Failed to load streamed level package %s by name '%s'. %s", PackageName.UTF8().c_str(), packageName.UTF8().c_str(), e.what());
    return false;
  }
  catch (...)
  {
    LogW("Failed to load streamed level package %s by name '%s'", PackageName.UTF8().c_str(), packageName.UTF8().c_str());
    return false;
  }
  LogW("Failed to load streamed level package %s by name '%s'", PackageName.UTF8().c_str(), packageName.UTF8().c_str());
  return false;
}

bool ULevelStreamingVolume::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, StreamingLevels))
  {
    StreamingLevelsProperty = property;
    for (FPropertyValue* value : property->Value->GetArray())
    {
      StreamingLevels.push_back(Cast<ULevelStreaming>(GetPackage()->GetObject(value->GetObjectIndex(), false)));
    }
    return true;
  }
  return false;
}

void ULevelStreamingVolume::PostLoad()
{
  Super::PostLoad();
  for (ULevelStreaming* level : StreamingLevels)
  {
    LoadObject(level);
  }
}

bool US1LevelStreamingDistance::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return true;
  }
  if (PROP_IS(property, ZoneNumberX))
  {
    ZoneNumberXProperty = property;
    ZoneNumberX = property->Value->GetInt();
    return true;
  }
  if (PROP_IS(property, ZoneNumberY))
  {
    ZoneNumberYProperty = property;
    ZoneNumberY = property->Value->GetInt();
    return true;
  }
  return false;
}
