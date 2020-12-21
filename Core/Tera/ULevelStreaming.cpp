#include "ULevelStreaming.h"
#include "FPackage.h"
#include "Cast.h"

#include <Utils/ALog.h>

bool ULevelStreaming::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
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
  // Try to load all S1 level by XY coords(except the S1Void)
  auto pos = PackageName.FindLastOf('_');
  if (pos != std::string::npos)
  {
    // Subtract 1 from the Y coordinate of the package zone
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
      FString newName = PackageName.Substr(0, pos + 1) + std::to_string(address);
      LogI("Loading the streaming level %s by custom ZoneXY %s", PackageName.UTF8().c_str(), newName.UTF8().c_str());
      if (LoadLevelPackage(PackageName.Substr(0, pos + 1) + std::to_string(address)))
      {
        // OK
        return;
      }
      LogI("Retring with the default name...");
    }
  }
  LoadLevelPackage(PackageName);
}

bool ULevelStreaming::LoadLevelPackage(const FString& packageName)
{
  if (packageName.Empty() || packageName == NAME_None)
  {
    return false;
  }
  try
  {
    if (std::shared_ptr<FPackage> package = FPackage::GetPackageNamed(packageName.String()))
    {
      package->Load();
      Level = Cast<ULevel>(package->GetObject(INDEX_NONE, "PersistentLevel", ULevel::StaticClassName()));
      GetPackage()->RetainPackage(package);
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
  SUPER_REGISTER_PROP();
  REGISTER_INT_PROP(ZoneNumberX);
  REGISTER_INT_PROP(ZoneNumberY);
  return false;
}

void US1LevelStreamingDistance::PostLoad()
{
  if (ZoneNumberXProperty && ZoneNumberYProperty)
  {
    auto pos = PackageName.FindLastOf('_');
    FString newName = PackageName.Substr(0, pos + 1) + std::to_string(ZoneNumberX) + std::to_string(ZoneNumberY);
    LogI("Loading the S1 streaming level %s by ZoneXY %s", PackageName.UTF8().c_str(), newName.UTF8().c_str());
    if (LoadLevelPackage(newName))
    {
      // OK
      return;
    }
    LogI("Retring with a different name...");
  }
  // Use default implementation (e.g., S1Void)
  Super::PostLoad();
}
