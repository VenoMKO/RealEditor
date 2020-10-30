#include "ULevelStreaming.h"
#include "FPackage.h"
#include "Cast.h"

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
  // TODO: refactor
  if (PackageName.Size() && PackageName != NAME_None)
  {
    try
    {
      FString originalPackageName = PackageName;
      bool retried = false;
retry:
      if (std::shared_ptr<FPackage> package = FPackage::GetPackageNamed(PackageName.String()))
      {
        package->Load();
        Level = Cast<ULevel>(package->GetObject(INDEX_NONE, "PersistentLevel", ULevel::StaticClassName()));
        GetPackage()->RetainPackage(package);
        FPackage::UnloadPackage(package);
      }
      else if (!retried)
      {
        retried = true;
        auto pos = PackageName.FindLastOf('_');
        if (pos != std::string::npos)
        {
          if (US1LevelStreamingDistance* s1this = Cast<US1LevelStreamingDistance>(this))
          {
            if (s1this->ZoneNumberXProperty && s1this->ZoneNumberYProperty)
            {
              // Use zone coordinates for proper package name (e.g. Zone_XXYY)
              PackageName = PackageName.Substr(0, pos + 1) + std::to_string(s1this->ZoneNumberX) + std::to_string(s1this->ZoneNumberY);
              goto retry;
            }
            else
            {
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
                PackageName = PackageName.Substr(0, pos + 1) + std::to_string(address);
                goto retry;
              }
            }
          }
          else
          {
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
              PackageName = PackageName.Substr(0, pos + 1) + std::to_string(address);
              goto retry;
            }
          }
        }
        
      }
      else
      {
        PackageName = originalPackageName;
      }
    }
    catch (...)
    {
    }
  }
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
