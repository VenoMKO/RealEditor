#include "FPackage.h"
#include "ALog.h"
#include "FStream.h"
#include "FObjectResource.h"
#include "UObject.h"
#include "UClass.h"
#include "UMetaData.h"
#include "UPersistentCookerData.h"
#include "UProperty.h"
#include "UTexture.h"
#include "Cast.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <array>
#include <ppl.h>

const char* PackageMapperName = "PkgMapper";
const char* CompositePackageMapperName = "CompositePackageMapper";
const char* ObjectRedirectorMapperName = "ObjectRedirectorMapper";
const char* PackageListName = "DirCache.re";
const char* PersistentDataName = "GlobalPersistentCookerData";

const char Key1[] = { 12, 6, 9, 4, 3, 14, 1, 10, 13, 2, 7, 15, 0, 8, 5, 11 };
const char Key2[] = { 'G', 'e', 'n', 'e', 'r', 'a', 't', 'e', 'P', 'a', 'c', 'k', 'a', 'g', 'e', 'M', 'a', 'p', 'p', 'e', 'r' };

const std::vector<FString> DefaultClassPackageNames = { "Core.u", "Engine.u", "S1Game.u", "GameFramework.u", "GFxUI.u" };

FString FPackage::RootDir;
std::recursive_mutex FPackage::PackagesMutex;
std::vector<std::shared_ptr<FPackage>> FPackage::LoadedPackages;
std::vector<std::shared_ptr<FPackage>> FPackage::DefaultClassPackages;
std::vector<FString> FPackage::DirCache;
std::unordered_map<FString, FString> FPackage::TfcCache;
std::unordered_map<FString, FString> FPackage::PkgMap;
std::unordered_map<FString, FString> FPackage::ObjectRedirectorMap;
std::unordered_map<FString, FCompositePackageMapEntry> FPackage::CompositPackageMap;
std::unordered_map<FString, std::vector<FString>> FPackage::CompositPackageList;
std::unordered_map<FString, FBulkDataInfo> FPackage::BulkDataMap;
std::unordered_map<FString, FTextureFileCacheInfo> FPackage::TextureCacheMap;
std::unordered_map<FString, std::unordered_map<FString, AMetaDataEntry>> FPackage::MetaData;
std::mutex FPackage::ClassMapMutex;
std::unordered_map<FString, UObject*> FPackage::ClassMap;
std::unordered_set<FString> FPackage::MissingClasses;
std::mutex FPackage::MissingPackagesMutex;
std::vector<FString> FPackage::MissingPackages;

uint16 FPackage::CoreVersion = 0;

void BuildPackageList(const FString& path, std::vector<FString>& dirCache, std::unordered_map<FString, FString>& tfcCache)
{
  std::filesystem::path fspath(path.WString());
  dirCache.resize(0);
  std::vector<std::filesystem::path> tmpPaths;
  std::unordered_map<std::string, std::filesystem::path> tmpTfcPaths;
  for (auto& p : std::filesystem::recursive_directory_iterator(fspath))
  {
    if (!p.is_regular_file() || !p.file_size())
    {
      continue;
    }
    std::filesystem::path itemPath = p.path();
    std::string ext = itemPath.extension().string();
    if (!_stricmp(ext.c_str(), ".gpk") || !_stricmp(ext.c_str(), ".gmp") || !_stricmp(ext.c_str(), ".upk") || !_stricmp(ext.c_str(), ".u"))
    {
      std::error_code err;
      std::filesystem::path rel = std::filesystem::relative(itemPath, fspath, err);
      if (!err)
      {
        tmpPaths.push_back(rel);
      }
    }
    else if (!_stricmp(ext.c_str(), ".tfc"))
    {
      std::error_code err;
      std::filesystem::path rel = std::filesystem::relative(itemPath, fspath, err);
      if (!err)
      {
        tmpTfcPaths[itemPath.filename().replace_extension().string()] = rel;
      }
    }
  }

  std::sort(tmpPaths.begin(), tmpPaths.end(), [](std::filesystem::path& a, std::filesystem::path& b) {
    auto tA = a.filename().replace_extension();
    auto tB = b.filename().replace_extension();
    return tA.wstring() < tB.wstring();
  });

  
  for (const auto& path : tmpPaths)
  {
    dirCache.emplace_back(W2A(path.wstring()));
  }
  for (const auto& item : tmpTfcPaths)
  {
    tfcCache[item.first] = W2A(item.second.wstring());
  }
#if CACHE_S1GAME_CONTENTS
  std::filesystem::path listPath = fspath / PackageListName;
  FWriteStream s(listPath.wstring());
  s << dirCache;
  s << tfcCache;
#endif
}

void EncryptMapper(const FString& decrypted, std::vector<char>& encrypted)
{
  size_t size = decrypted.Size();
  size_t offset = 0;
  encrypted.resize(size);
  for (offset = 0; offset < size; ++offset)
  {
    encrypted[offset] = decrypted[offset] ^ Key2[offset % sizeof(Key2)];
  }

  {
    size_t a = 1;
    size_t b = size - 1;
    for (offset = (size / 2 + 1) / 2; offset; --offset, a += 2, b -= 2)
    {
      std::swap(encrypted[a], encrypted[b]);
    }
  }

  std::array<char, sizeof(Key1)> tmp;
  for (size_t offset = 0; offset + sizeof(Key1) <= size; offset += sizeof(Key1))
  {
    memcpy(&tmp[0], &encrypted[offset], sizeof(Key1));
    for (size_t idx = 0; idx < sizeof(Key1); ++idx)
    {
      encrypted[offset + idx] = tmp[Key1[idx]];
    }
  }
}

void DecryptMapper(const std::filesystem::path& path, FString& decrypted)
{
  if (!std::filesystem::exists(path))
  {
    UThrow("File \"%s\" does not exist!", path.string().c_str());
  }
  std::vector<char> encrypted;
  size_t size = 0;

  {
    std::ifstream s(path.wstring(), std::ios::binary | std::ios::ate);
    if (!s.is_open())
    {
      UThrow("Can't open \"%s\"!", path.string().c_str());
    }
    s.seekg(0, std::ios_base::end);
    size = s.tellg();
    s.seekg(0, std::ios_base::beg);
    encrypted.resize(size);
    decrypted.Resize(size);
    s.read(&encrypted[0], size);
  }
  
  LogI("Decrypting \"%s\"", path.filename().string().c_str());

  size_t offset = 0;
  for (; offset + sizeof(Key1) <= size; offset += sizeof(Key1))
  {
    for (size_t idx = 0; idx < sizeof(Key1); ++idx)
    {
      decrypted[offset + idx] = encrypted[offset + Key1[idx]];
    }
  }
  for (; offset < size; ++offset)
  {
    decrypted[offset] = encrypted[offset];
  }

  {
    size_t a = 1;
    size_t b = size - 1;
    for (offset = (size / 2 + 1) / 2; offset; --offset, a += 2, b -= 2)
    {
      std::swap(decrypted[a], decrypted[b]);
    }
  }

  for (offset = 0; offset < size; ++offset)
  {
    decrypted[offset] ^= Key2[offset % sizeof(Key2)];
  }

  if ((*(wchar*)decrypted.C_str()) == 0xFEFF)
  {
    FString utfstr = ((wchar*)decrypted.C_str()) + 1;
    decrypted = utfstr;
  }
}

void FPackage::SetRootPath(const FString& path)
{
  RootDir = path;
#if CACHE_S1GAME_CONTENTS
  std::filesystem::path listPath = std::filesystem::path(path.WString()) / PackageListName;
  FReadStream s(listPath.wstring());
  if (s.IsGood())
  {
    s << DirCache;
    s << TfcCache;
    return;
  }
#endif
  LogI("Building directory cache: \"%s\"", path.C_str());
  BuildPackageList(path, DirCache, TfcCache);
  LogI("Done. Found %ld packages", DirCache.size());
}

void FPackage::SetMetaData(const std::unordered_map<FString, std::unordered_map<FString, AMetaDataEntry>>& meta)
{
  MetaData = meta;
}

FBulkDataInfo* FPackage::GetBulkDataInfo(const FString& bulkDataName)
{
  if (BulkDataMap.count(bulkDataName))
  {
    return &BulkDataMap[bulkDataName];
  }
  return nullptr;
}

FString FPackage::GetTextureFileCachePath(const FString& tfcName)
{
  if (TfcCache.count(tfcName))
  {
    return RootDir.FStringByAppendingPath(TfcCache[tfcName]);
  }
  return FString();
}

const std::unordered_map<FString, FCompositePackageMapEntry>& FPackage::GetCompositePackageMap()
{
  return CompositPackageMap;
}

const std::unordered_map<FString, std::vector<FString>>& FPackage::GetCompositePackageList()
{
  return CompositPackageList;
}

FString FPackage::GetCompositePackageMapPath()
{
  std::filesystem::path encryptedPath = std::filesystem::path(RootDir.WString()) / "CookedPC" / CompositePackageMapperName;
  encryptedPath.replace_extension(".dat");
  return encryptedPath.wstring();
}

FString FPackage::GetObjectCompositePath(const FString& path)
{
  if (path.Size() && PkgMap.count(path.ToUpper()))
  {
    return PkgMap.at(path.ToUpper());
  }
  return FString();
}

void FPackage::UpdateDirCache()
{
  LogI("Building directory cache: \"%s\"", RootDir.C_str());
  {
    std::scoped_lock<std::mutex> l(MissingPackagesMutex);
    MissingPackages.clear();
  }
  BuildPackageList(RootDir, DirCache, TfcCache);
  LogI("Done. Found %ld packages", DirCache.size());
}

void FPackage::CreateCompositeMod(const std::vector<FString>& items, const FString& destination, FString name, FString author)
{
  std::vector<FString> objects;
  for (const FString& path : items)
  {
    FReadStream s(path);
    FPackageSummary sum;
    s << sum;
    if (!s.IsGood())
    {
      UThrow("Failed to read package %s.", path.Filename().C_str());
    }
    if (!sum.FolderName.StartWith("MOD:"))
    {
      UThrow("Package %s has no composite info! Try to resave it from the original.", sum.PackageName.C_str());
    }
    FString objName = sum.FolderName.Substr(4);
    if (objName.Empty())
    {
      UThrow("Package %s has no composite info! Try to resave it from the original.", sum.PackageName.C_str());
    }
    for (int32 idx = 0; idx < objects.size(); ++idx)
    {
      if (objects[idx] == objName)
      {
        UThrow("%s and %s are modifying the same composite package!", path.Filename().C_str(), items[idx].Filename().C_str());
      }
    }
    objects.push_back(objName);
  }

  std::vector<FILE_OFFSET> offsets;
  FWriteStream write(destination);
  for (const FString& path : items)
  {
    FReadStream read(path);
    FILE_OFFSET size = read.GetSize();
    void* data = malloc(size);
    read.SerializeBytes(data, size);
    offsets.push_back(write.GetPosition());
    write.SerializeBytes(data, size);
    free(data);
  }

  // Save metadata

  // MAGIC
  
  // Author
  // Name
  // Container(Filename)
  // Composite offsets...

  // MAGIC

  // Region-lock
  // Author offset
  // Name offset
  // Container offset
  // Composite offsets offset
  // Composite offsets count
  // Meta size

  // MAGIC
  // EOF

  uint32 tmp = PACKAGE_MAGIC;
  uint32 metaSize = write.GetPosition();
  write << tmp;

  FILE_OFFSET authorOffset = write.GetPosition();
  write << author;
  FILE_OFFSET nameOffset = write.GetPosition();
  write << name;
  FILE_OFFSET containerOffset = write.GetPosition();
  FString container = destination.Filename(false);
  write << container;
  
  FILE_OFFSET offsetsOffset = write.GetPosition();
  for (FILE_OFFSET offset : offsets)
  {
    write << offset;
  }
  
  tmp = PACKAGE_MAGIC;
  write << tmp;
  tmp = 0; // Region-lock (unused): 1 - direct object path search, 0 - incomplete object path search
  write << tmp;
  write << authorOffset;
  write << nameOffset;
  write << containerOffset;
  write << offsetsOffset;
  tmp = (uint32)offsets.size();
  write << tmp;
  FILE_OFFSET metaSizeOffset = write.GetPosition();
  write << metaSize;
  tmp = PACKAGE_MAGIC;
  write << tmp;

  metaSize = write.GetPosition() - metaSize;
  write.SetPosition(metaSizeOffset);
  write << metaSize;
}

std::vector<UClass*> FPackage::GetClasses()
{
  std::vector<UClass*> result;
  result.reserve(ClassMap.size());
  for (const auto& p : ClassMap)
  {
    if (UClass* cls = Cast<UClass>(p.second))
    {
      result.push_back(cls);
    }
  }
  return result;
}

uint16 FPackage::GetCoreVersion()
{
  return CoreVersion;
}

void FPackage::LoadPersistentData()
{
  if (std::shared_ptr<FPackage> package = GetPackageNamed(PersistentDataName))
  {
    package->Load();
    for (FObjectExport* exp : package->Exports)
    {
      if (exp->GetClassName() == UPersistentCookerData::StaticClassName())
      {
        if (UPersistentCookerData* data = Cast<UPersistentCookerData>(package->GetObject(exp->ObjectIndex, false)))
        {
          data->GetPersistentData(BulkDataMap, TextureCacheMap);
          break;
        }
      }
    }
    UnloadPackage(package);
  }
}

void FPackage::LoadClassPackage(const FString& name)
{
  LogI("Loading %s", name.C_str());
  if (auto package = GetPackageNamed(name))
  {
    package->AllowEdit = false;
    package->AllowForcedExportResolving = false;
    package->Load();
    DefaultClassPackages.push_back(package);
    if (name == "Core.u")
    {
      CoreVersion = package->GetFileVersion();
      if (CoreVersion == VER_TERA_CLASSIC)
      {
        UThrow("Real Editor does not support 32-bit Tera!");
      }
      LogI("Core version: %u/%u", package->GetFileVersion(), package->GetLicenseeVersion());
    }
    else if (package->GetFileVersion() != CoreVersion)
    {
      UThrow("Package %s has different version %d/%d ", name.C_str(), package->GetFileVersion(), package->GetLicenseeVersion());
    }

    UClass::CreateBuiltInClasses(package.get());

    // Load package in memory and create a stream
    FILE_OFFSET packageSize = package->Stream->GetSize();
    void* rawPackageData = malloc(packageSize);
    package->Stream->SerializeBytesAt(rawPackageData, 0, packageSize);

    MReadStream packageStream(rawPackageData, true, packageSize);
    packageStream.SetPackage(package.get());

    // Don't load referenced objects
    packageStream.SetLoadSerializedObjects(false);
    package->Stream->SetLoadSerializedObjects(false);

    // List of root classes
    std::vector<UObject*> classes;
    // List of root defaults\templates
    std::vector<UObject*> defaults;
    // Leftovers
    std::vector<UObject*> other;
    // Properties
    std::vector<UProperty*> properties;

    // Prepare object lists
    UMetaData* meta = nullptr;
    for (auto p : package->ExportObjects)
    {
      if (!p.second)
      {
        continue;
      }
      UObject* obj = p.second;
      
      if (obj->IsTemplate(RF_ClassDefaultObject))
      {
        defaults.push_back(obj);
      }
      else if (obj->GetClassName() == NAME_Class)
      {
        classes.push_back(obj);
        std::scoped_lock<std::mutex> l(ClassMapMutex);
        ClassMap[obj->GetObjectName()] = obj;
      }
      else if (obj->IsA(UMetaData::StaticClassName()))
      {
        meta = (UMetaData*)obj;
      }
      else
      {
        if (obj->IsA(UProperty::StaticClassName()))
        {
          properties.push_back((UProperty*)obj);
        }
        other.push_back(obj);
      }
    }

    // Load objects from child to parent to prevent possible stack overflow
    std::function<void(UObject*, std::vector<UObject*>&)> loader;
    loader = [&loader](UObject* obj, std::vector<UObject*>& children) {
      auto tmp = obj->GetInner();
      for (UObject* child : tmp)
      {
        loader(child, children);
      }
      children.push_back(obj);
    };

    // Serialize classes
    for (size_t idx = 0; idx < classes.size(); ++idx)
    {
      std::vector<UObject*> objects;
      loader(classes[idx], objects);
      MReadStream s(packageStream.GetAllocation(), false, packageSize);
      s.SetPackage(package.get());
      s.SetLoadSerializedObjects(false);
      for (UObject* obj : objects)
      {
        obj->Load(s);
      }
    }

    // Link fields
    for (size_t idx = 0; idx < classes.size(); ++idx)
    {
      if (UClass* cls = Cast<UClass>(classes[idx]))
      {
        cls->Link();
      }
    }

    // Serialize defaults
    for (size_t idx = 0; idx < defaults.size(); ++idx)
    {
      UObject* root = defaults[idx];
      MReadStream s(packageStream.GetAllocation(), false, packageSize);
      s.SetPackage(package.get());
      s.SetLoadSerializedObjects(false);
      if (root->GetInner().size())
      {
        std::vector<UObject*> objects;
        loader(root, objects);
        for (UObject* obj : objects)
        {
          obj->Load(s);
        }
      }
      else
      {
        root->Load(s);
      }
    }

    // It's safe to load references now
    package->Stream->SetLoadSerializedObjects(true);
    packageStream.SetLoadSerializedObjects(true);

    for (size_t idx = 0; idx < other.size(); ++idx)
    {
      UObject* root = other[idx];
      MReadStream s(packageStream.GetAllocation(), false, packageSize);
      s.SetPackage(package.get());
      s.SetLoadSerializedObjects(false);
      if (root->GetInner().size())
      {
        std::vector<UObject*> objects;
        loader(root, objects);
        for (UObject* obj : objects)
        {
          obj->Load(s);
        }
      }
      else
      {
        root->Load(s);
      }
    }

    if (meta)
    {
      meta->Load();
      const auto& objMap = meta->GetObjectMetaDataMap();
      for (const auto& pair : objMap)
      {
        if (UField* field = Cast<UField>(pair.first))
        {
          for (const auto& info : pair.second)
          {
            if (info.first == "ToolTip")
            {
              field->SetToolTip(info.second);
              break;
            }
          }
        }
      }
    }

    for (size_t idx = 0; idx < properties.size(); ++idx)
    {
      UProperty* p = properties.at(idx);
      const FString propertyName = p->GetObjectName();
      UObject* outer = p->GetOuter();
      std::vector<UObject*> parents;

      while (outer->GetOuter())
      {
        parents.push_back(outer);
        outer = outer->GetOuter();
      }
      const FString className = outer->GetObjectName();
      if (parents.size() && parents.front()->IsA(UFunction::StaticClassName()))
      {
        continue;
      }

      if (parents.size())
      {
        FString key = className + ":" + parents.back()->GetObjectName();
        if (MetaData.count(key))
        {
          if (MetaData.at(key).count(propertyName))
          {
            const AMetaDataEntry& entry = MetaData.at(key).at(propertyName);
            if (entry.Name.Size())
            {
              p->DisplayName = entry.Name;
            }
            if (entry.Tooltip.Size())
            {
              p->SetToolTip(entry.Tooltip);
            }
            continue;
          }
        }
      }

      if (MetaData.count(outer->GetObjectName().String()))
      {
        const FString& key = className;
        if (MetaData[key].count(propertyName))
        {
          const AMetaDataEntry& entry = MetaData.at(key).at(propertyName);
          if (entry.Name.Size())
          {
            p->DisplayName = entry.Name;
          }
          if (entry.Tooltip.Size())
          {
            p->SetToolTip(entry.Tooltip);
          }
        }
      }
    }

#ifdef _DEBUG
    for (FObjectExport* exp : package->Exports)
    {
      UObject* obj = package->GetObject(exp->ObjectIndex, false);
      DBreakIf(!obj || !obj->IsLoaded());
    }
#endif
  }
  else
  {
    UThrow("Failed to load: %s!", name.C_str());
  }
}

void FPackage::UnloadDefaultClassPackages()
{
  ClassMap.clear();
  MissingClasses.clear();
  for (auto package : DefaultClassPackages)
  {
    UnloadPackage(package);
  }
  DefaultClassPackages.clear();
}

void FPackage::LoadPkgMapper(bool rebuild)
{
  PkgMap.clear();
  std::filesystem::path storagePath = std::filesystem::path(RootDir.WString()) / PackageMapperName;
  storagePath.replace_extension(".re");
  std::filesystem::path encryptedPath = std::filesystem::path(RootDir.WString()) / "CookedPC" / PackageMapperName;
  encryptedPath.replace_extension(".dat");
  LogI("Reading %s storage...", PackageMapperName);

  uint64 fts = GetFileTime(encryptedPath.wstring());
  if (std::filesystem::exists(storagePath) && !rebuild)
  {
    FReadStream rs(storagePath.wstring());
    uint64 ts = 0;
    rs << ts;
    if (fts == ts || !fts)
    {
      rs << PkgMap;
      return;
    }
    else
    {
      LogW("%s storage is outdated! Updating...", PackageMapperName);
    }
  }

  if (!std::filesystem::exists(encryptedPath))
  {
    UThrow("Failed to open %s", encryptedPath.string().c_str());
  }

  FString buffer;
  DecryptMapper(encryptedPath, buffer);

  size_t pos = 0;
  size_t prevPos = 0;
  while ((pos = buffer.Find('|', prevPos)) != std::string::npos)
  {
    size_t sepPos = buffer.Find(',', prevPos);
    if (sepPos == std::string::npos || sepPos > pos)
    {
      UThrow("%s is corrupted!", PackageMapperName);
    }
    FString key = buffer.Substr(prevPos, sepPos - prevPos).ToUpper();
    FString value = buffer.Substr(sepPos + 1, pos - sepPos - 1);
    PkgMap.emplace(key, value);
    pos++;
    std::swap(pos, prevPos);
  }

  LogI("Saving %s storage", PackageMapperName);
  FWriteStream ws(storagePath.wstring());
  ws << fts;
  ws << PkgMap;

#if DUMP_MAPPERS
  std::filesystem::path debugPath = storagePath;
  debugPath.replace_extension(".txt");
  std::ofstream os(debugPath.wstring());
  os.write(&buffer[0], buffer.Size());
#endif
}

void FPackage::LoadCompositePackageMapper(bool rebuild)
{
  {
    std::scoped_lock<std::mutex> l(MissingPackagesMutex);
    MissingPackages.clear();
  }
  CompositPackageMap.clear();
  CompositPackageList.clear();
  std::filesystem::path encryptedPath = std::filesystem::path(RootDir.WString()) / "CookedPC" / CompositePackageMapperName;
  encryptedPath.replace_extension(".dat");

#if CACHE_COMPOSITE_MAP
  LogI("Reading %s storage...", CompositePackageMapperName);
  std::filesystem::path storagePath = std::filesystem::path(RootDir.WString()) / CompositePackageMapperName;
  storagePath.replace_extension(".re");
  uint64 fts = GetFileTime(encryptedPath.wstring());
  if (std::filesystem::exists(storagePath) && !rebuild)
  {
    FReadStream s(storagePath.wstring());
    uint64 ts = 0;
    s << ts;
    if (fts == ts || !fts)
    {
      s << CompositPackageMap;
      for (auto pair : CompositPackageMap)
      {
        CompositPackageList[pair.second.FileName.ToUpper()].push_back(pair.first);
      }
      return;
    }
    else
    {
      LogW("%s storage is outdated! Updating...", CompositePackageMapperName);
    }
  }
#endif

  FString buffer;
  DecryptMapper(encryptedPath, buffer);

  size_t pos = 0;
  size_t posEnd = 0;
  while ((pos = buffer.Find('?', posEnd)) != std::string::npos)
  {
    FString fileName = buffer.Substr(posEnd, pos - posEnd);
    posEnd = buffer.Find('!', pos);
    do
    {
      pos++;
      FCompositePackageMapEntry entry;
      entry.FileName = fileName;

      size_t elementEnd = buffer.Find(',', pos);
      entry.ObjectPath = buffer.Substr(pos, elementEnd - pos);
      std::swap(pos, elementEnd);
      pos++;

      elementEnd = buffer.Find(',', pos);
      FString packageName = buffer.Substr(pos, elementEnd - pos);
      std::swap(pos, elementEnd);
      pos++;

      elementEnd = buffer.Find(',', pos);
      entry.Offset = (FILE_OFFSET)std::stoul(buffer.Substr(pos, elementEnd - pos).String());
      std::swap(pos, elementEnd);
      pos++;

      elementEnd = buffer.Find(',', pos);
      entry.Size = (FILE_OFFSET)std::stoul(buffer.Substr(pos, elementEnd - pos).String());
      std::swap(pos, elementEnd);
      pos++;

      DBreakIf(CompositPackageMap.count(packageName));
      CompositPackageMap[packageName] = entry;
      CompositPackageList[fileName].push_back(packageName);
    } while (buffer.Find('|', pos) < posEnd - 1);
    pos++;
    posEnd++;
  }

#if CACHE_COMPOSITE_MAP
  LogI("Saving %s storage", CompositePackageMapperName);
  FWriteStream s(storagePath.wstring());
  s << fts;
  s << CompositPackageMap;
#endif

#if DUMP_MAPPERS
  std::filesystem::path debugPath = std::filesystem::path(RootDir.WString()) / CompositePackageMapperName;
  debugPath.replace_extension(".txt");
  std::ofstream os(debugPath.wstring());
  os.write(&buffer[0], buffer.Size());
#endif
}

void FPackage::LoadObjectRedirectorMapper(bool rebuild)
{
  ObjectRedirectorMap.clear();
  std::filesystem::path storagePath = std::filesystem::path(RootDir.WString()) / ObjectRedirectorMapperName;
  storagePath.replace_extension(".re");
  std::filesystem::path encryptedPath = std::filesystem::path(RootDir.WString()) / "CookedPC" / ObjectRedirectorMapperName;
  encryptedPath.replace_extension(".dat");
  LogI("Reading %s storage...", ObjectRedirectorMapperName);
  uint64 fts = GetFileTime(encryptedPath.wstring());
  if (std::filesystem::exists(storagePath) && !rebuild)
  {
    FReadStream rs(storagePath.wstring());
    uint64 ts = 0;
    rs << ts;
    if (fts == ts || !fts)
    {
      rs << ObjectRedirectorMap;
      return;
    }
    else
    {
      LogW("%s storage is outdated! Updating...", ObjectRedirectorMapperName);
    }
  }

  FString buffer;
  DecryptMapper(encryptedPath, buffer);
  
  size_t pos = 0;
  size_t prevPos = 0;
  while ((pos = buffer.Find('|', prevPos)) != std::string::npos)
  {
    size_t sepPos = buffer.Find(',', prevPos);
    if (sepPos == std::string::npos || sepPos > pos)
    {
      UThrow("%s is corrupted!", ObjectRedirectorMapperName);
    }
    FString key = buffer.Substr(prevPos, sepPos - prevPos);
    FString value = buffer.Substr(sepPos + 1, pos - sepPos - 1);
    ObjectRedirectorMap.emplace(key, value);
    pos++;
    std::swap(pos, prevPos);
  }
  
  LogI("Saving %s storage", ObjectRedirectorMapperName);
  FWriteStream ws(storagePath.wstring());
  ws << fts;
  ws << ObjectRedirectorMap;

#if DUMP_MAPPERS
  std::filesystem::path debugPath = storagePath;
  debugPath.replace_extension(".txt");
  std::ofstream os(debugPath.wstring(), std::ios::out | std::ios::binary | std::ios::trunc);
  os.write(&buffer[0], buffer.Size());
  os.close();
#endif
}

std::shared_ptr<FPackage> FPackage::GetPackage(const FString& path)
{
  std::shared_ptr<FPackage> found = nullptr;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
    for (auto package : LoadedPackages)
    {
      if (package->GetSourcePath() == path)
      {
        found = package;
        break;
      }
    }
    if (found)
    {
      LoadedPackages.push_back(found);
      return found;
    }
  }
  
  LogI("Opening package: %s", path.String().c_str());
  FStream *stream = new FReadStream(path);
  if (!stream->IsGood())
  {
    delete stream;
    UThrow("Couldn't open the file: %s!", path.FilenameString(true).c_str());
    // Shut up static analyzer
    return nullptr;
  }
  FPackageSummary sum;
  sum.SourcePath = path;
  sum.DataPath = path;
  sum.PackageName = std::filesystem::path(path.WString()).filename().wstring();
  (*stream) << sum;
  if (CoreVersion && sum.GetFileVersion() != CoreVersion)
  {
    if (sum.GetFileVersion() == VER_TERA_CLASSIC)
    {
      UThrow("Real Editor can't open 32-bit packages!");
    }
    UThrow("%s version (%d/%d) differs from your game version(%d)", sum.PackageName.C_str(), sum.GetFileVersion(), sum.GetLicenseeVersion(), CoreVersion);
  }
  if (sum.CompressedChunks.size())
  {
    FILE_OFFSET startOffset = INT_MAX;
    FILE_OFFSET totalDecompressedSize = 0;
    void** compressedChunksData = new void*[sum.CompressedChunks.size()];
    {
      uint32 idx = 0;
      for (FCompressedChunk& chunk : sum.CompressedChunks)
      {
        compressedChunksData[idx] = malloc(chunk.CompressedSize);
        stream->SetPosition(chunk.CompressedOffset);
        stream->SerializeBytes(compressedChunksData[idx], chunk.CompressedSize);
        totalDecompressedSize += chunk.DecompressedSize;
        if (chunk.DecompressedOffset < startOffset)
        {
          startOffset = chunk.DecompressedOffset;
        }
        idx++;
      }
    }

    std::filesystem::path decompressedPath = std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
    sum.DataPath = W2A(decompressedPath.wstring());
    LogI("Decompressing package %s to %s", sum.PackageName.C_str(), decompressedPath.string().c_str());
    FStream* tempStream = new FWriteStream(sum.DataPath.WString());

    sum.OriginalPackageFlags = sum.PackageFlags;
    sum.OriginalCompressionFlags = sum.CompressionFlags;
    sum.PackageFlags &= ~PKG_StoreCompressed;
    sum.CompressionFlags = COMPRESS_None;

    std::vector<FCompressedChunk> chunks;
    std::swap(sum.CompressedChunks, chunks);
    auto tmpSize = sum.SourceSize;
    (*tempStream) << sum;
    sum.SourceSize = tmpSize;

    uint8* decompressedData = (uint8*)malloc(totalDecompressedSize);
    try
    {
      concurrency::parallel_for(size_t(0), size_t(chunks.size()), [&chunks, compressedChunksData, decompressedData, startOffset](size_t idx) {
        const FCompressedChunk& chunk = chunks[idx];
        uint8* dst = decompressedData + chunk.DecompressedOffset - startOffset;
        LZO::Decompress(compressedChunksData[idx], chunk.CompressedSize, dst, chunk.DecompressedSize);
      });
    }
    catch (const std::exception& e)
    {
      for (size_t idx = 0; idx < chunks.size(); ++idx)
      {
        free(compressedChunksData[idx]);
      }
      delete[] compressedChunksData;
      free(decompressedData);
      delete tempStream;
      delete stream;
      throw e;
    }
    for (size_t idx = 0; idx < chunks.size(); ++idx)
    {
      free(compressedChunksData[idx]);
    }
    delete[] compressedChunksData;
    tempStream->SerializeBytes(decompressedData, totalDecompressedSize);
    free(decompressedData);
    delete tempStream;
    delete stream;

    // Read decompressed header
    stream = new FReadStream(sum.DataPath);
    try
    {
      (*stream) << sum;
    }
    catch (const std::exception& e)
    {
      delete stream;
      throw e;
    }
    sum.PackageName = std::filesystem::path(path.WString()).filename().wstring();
  }
  
  delete stream;
  std::shared_ptr<FPackage> result = nullptr;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
    result = LoadedPackages.emplace_back(new FPackage(sum));
  }
  return result;
}

std::shared_ptr<FPackage> FPackage::GetPackageNamed(const FString& name, FGuid guid)
{
  {
    std::scoped_lock<std::mutex> l(MissingPackagesMutex);
    if (std::find(MissingPackages.begin(), MissingPackages.end(), name) != MissingPackages.end())
    {
      return nullptr;
    }
  }

  bool anyPackageFound = false;
  std::shared_ptr<FPackage> found = nullptr;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
    for (auto package : LoadedPackages)
    {
      if (package->GetPackageName() == name)
      {
        anyPackageFound = true;
        if (guid.IsValid())
        {
          if (package->GetGuid() == guid)
          {
            found = package;
            break;
          }
        }
        else
        {
          found = package;
          break;
        }
      }
    }
    if (found)
    {
      return LoadedPackages.emplace_back(found);
    }
  }
  
  // Check and load if the package is a composit package
  if (CoreVersion > VER_TERA_CLASSIC && CompositPackageMap.count(name))
  {
    const FCompositePackageMapEntry& entry = CompositPackageMap[name];
    FString packagePath;
    for (FString& path : DirCache)
    {
      std::wstring filename = path.FilenameWString();
      if (filename.size() < entry.FileName.Size())
      {
        continue;
      }
      std::wstring tmp = entry.FileName.WString();
      if (std::mismatch(tmp.begin(), tmp.end(), filename.begin()).first == tmp.end())
      {
        packagePath = RootDir.FStringByAppendingPath(path);
        break;
      }
    }
    if (packagePath.Size())
    {
      LogI("Reading composite package %s from %s...", name.C_str(), entry.FileName.C_str());
      void* rawData = malloc(entry.Size);
      {
        FReadStream rs(packagePath);
        rs.SetPosition(entry.Offset);
        rs.SerializeBytes(rawData, entry.Size);
        if (!rs.IsGood())
        {
          UThrow("Failed to read %s", name.C_str());
        }
      }
      std::filesystem::path tmpPath = std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
      LogI("Writing composite package %s to %s...", name.C_str(), tmpPath.string().c_str());
      {
        FWriteStream ws(tmpPath);
        ws.SerializeBytes(rawData, entry.Size);
        if (!ws.IsGood())
        {
          UThrow("Failed to save composit package %s to %s", name.C_str(), tmpPath.string().c_str());
        }
      }
      free(rawData);
      std::shared_ptr<FPackage> package = GetPackage(tmpPath.wstring());
      package->CompositeDataPath = tmpPath.wstring();
      package->CompositeSourcePath = packagePath.WString();
      package->Summary.PackageName = name;
      package->Composite = true;
      package->AllowForcedExportResolving = false;
      return package;
    }
  }

  std::wstring wname = name.WString();
  for (FString& path : DirCache)
  {
    std::wstring filename = path.FilenameWString();
    if (filename.size() < wname.size())
    {
      continue;
    }
    if (std::mismatch(wname.begin(), wname.end(), filename.begin()).first == wname.end())
    {
      if (auto package = GetPackage(RootDir.FStringByAppendingPath(path)))
      {
        anyPackageFound = true;
        if (!guid.IsValid() || guid == package->GetGuid())
        {
          return package;
        }
        UnloadPackage(package);
      }
    }
  }

  // If we have not found any packages with similar name regardless guid
  if (!anyPackageFound)
  {
    std::scoped_lock<std::mutex> l(MissingPackagesMutex);
    MissingPackages.push_back(name);
  }
  return nullptr;
}

void FPackage::UnloadPackage(std::shared_ptr<FPackage> package)
{
  if (!package)
  {
    return;
  }
  bool lastPackageRef = false;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
    for (size_t idx = 0; idx < LoadedPackages.size(); ++idx)
    {
      if (LoadedPackages[idx].get() == package.get())
      {
        if (!lastPackageRef)
        {
          LoadedPackages.erase(LoadedPackages.begin() + idx);
          lastPackageRef = true;
          idx--;
        }
        else
        {
          // There are more refs. No need to continue
          lastPackageRef = false;
          break;
        }
      }
    }
  }
  if (lastPackageRef)
  {
    LogI("Package %s unloaded", package->GetPackageName().C_str());
  }
}

FPackage::~FPackage()
{
  if (Stream)
  {
    delete Stream;
  }
  for (auto& obj : ExportObjects)
  {
    delete obj.second;
  }
  for (FObjectExport* exp : Exports)
  {
    delete exp;
  }
  for (FObjectImport* imp : Imports)
  {
    delete imp;
  }
  for (VObjectExport* exp : VExports)
  {
    delete exp;
  }
  for (FObjectThumbnailInfo* info : ThumbnailInfos)
  {
    delete info;
  }
  for (FObjectThumbnail* thumbnail : Thumbnails)
  {
    delete thumbnail;
  }
  
  for (auto& pkg : ExternalPackages)
  {
    UnloadPackage(pkg);
  }
  ExternalPackages.clear();
  if (Summary.DataPath != Summary.SourcePath)
  {
    std::filesystem::remove(std::filesystem::path(Summary.DataPath.WString()));
  }
  if (CompositeDataPath.Size())
  {
    std::filesystem::remove(std::filesystem::path(CompositeDataPath.WString()));
  }
}

void FPackage::Load()
{
#define CheckCancel() if (Cancelled.load()) { Loading.store(false); return; } //
  if (Ready.load() || Loading.load())
  {
    return;
  }
  Loading.store(true);
  Stream = new FReadStream(Summary.DataPath);
  Stream->SetPackage(this);
  FStream& s = GetStream();
  if (Summary.NamesOffset != s.GetPosition())
  {
    s.SetPosition(Summary.NamesOffset);
  }
  AllowForcedExportResolving = false;
  Names.clear();
  Summary.NamesSize = s.GetPosition();
  for (uint32 idx = 0; idx < Summary.NamesCount; ++idx)
  {
    FNameEntry& e = Names.emplace_back(FNameEntry());
    s << e;
    CheckCancel();
  }
  Summary.NamesSize = s.GetPosition() - Summary.NamesSize;

  if (Summary.ImportsOffset != s.GetPosition())
  {
    s.SetPosition(Summary.ImportsOffset);
  }
  Imports.clear();
  for (uint32 idx = 0; idx < Summary.ImportsCount; ++idx)
  {
    FObjectImport* imp = Imports.emplace_back(new FObjectImport(this));
    imp->ObjectIndex = -(PACKAGE_INDEX)idx - 1;
    s << *imp;
    CheckCancel();
  }

  if (Summary.ExportsOffset != s.GetPosition())
  {
    s.SetPosition(Summary.ExportsOffset);
  }
  Exports.clear();
  for (uint32 idx = 0; idx < Summary.ExportsCount; ++idx)
  {
    FObjectExport* exp = Exports.emplace_back(new FObjectExport(this));
    exp->ObjectIndex = (PACKAGE_INDEX)idx + 1;
    s << *exp;
    CheckCancel();
#ifdef _DEBUG
    exp->ClassNameValue = exp->GetClassName();
#endif
  }
  
  for (FObjectExport* exp : Exports)
  {
    ExportObjects[exp->ObjectIndex] = UObject::Object(exp);
  }

  if (Summary.DependsOffset != s.GetPosition())
  {
    s.SetPosition(Summary.DependsOffset);
  }
  Depends.clear();
  for (uint32 idx = 0; idx < Summary.ExportsCount; ++idx)
  {
    std::vector<int>& arr = Depends.emplace_back(std::vector<int>());
    s << arr;
    CheckCancel();
  }

  if (Summary.ThumbnailTableOffset)
  {
    if (Summary.ThumbnailTableOffset != s.GetPosition())
    {
      s.SetPosition(Summary.ThumbnailTableOffset);
    }

    int32 thumbnailCount = 0;
    s << thumbnailCount;

    ThumbnailInfos.resize(thumbnailCount);
    for (int32 idx = 0; idx < thumbnailCount; ++idx)
    {
      ThumbnailInfos[idx] = new FObjectThumbnailInfo;
      s << *ThumbnailInfos[idx];
    }

    Thumbnails.resize(thumbnailCount);
    for (int32 idx = 0; idx < thumbnailCount; ++idx)
    {
      Thumbnails[idx] = new FObjectThumbnail;
      s.SetPosition(ThumbnailInfos[idx]->Offset);
      s << *Thumbnails[idx];
    }
  }

  if (Summary.ImportExportGuidsOffset)
  {
    if (Summary.ImportExportGuidsOffset != s.GetPosition())
    {
      s.SetPosition(Summary.ImportExportGuidsOffset);
    }
    DBreakIf(Summary.ImportGuidsCount || Summary.ExportGuidsCount);
    ImportGuids.clear();
    ImportGuids.resize(Summary.ImportGuidsCount);
    for (int32 idx = 0; idx < Summary.ImportGuidsCount; ++idx)
    {
      s << ImportGuids[idx];
    }

    ExportGuids.clear();
    for (int32 idx = 0; idx < Summary.ExportGuidsCount; ++idx)
    {
      FGuid objectGuid;
      PACKAGE_INDEX exportIndex = 0;
      s << objectGuid << exportIndex;
      ExportGuids[objectGuid] = GetExportObject(exportIndex);
    }
  }

  for (uint32 index = 0; index < Summary.ExportsCount; ++index)
  {
    FObjectExport* exp = Exports[index];
#ifdef _DEBUG
    exp->Path = exp->GetObjectPath();
#endif
    if (exp->OuterIndex)
    {
      ExportObjects[exp->ObjectIndex]->SetOuter(ExportObjects[exp->OuterIndex]);
      ExportObjects[exp->OuterIndex]->AddInner(ExportObjects[exp->ObjectIndex]);

      FObjectExport* outer = GetExportObject(exp->OuterIndex);
      outer->Inner.push_back(exp);
      exp->Outer = outer;
    }
    else
    {
      RootExports.push_back(exp);
    }
    CheckCancel();
    ObjectNameToExportMap[exp->GetObjectName()].push_back(exp);
  }

  for (uint32 idx = 0; idx < Summary.ImportsCount; ++idx)
  {
    CheckCancel();
    FObjectImport* imp = Imports[idx];
#ifdef _DEBUG
    imp->Path = imp->GetObjectPath();
#endif
    if (imp->OuterIndex)
    {
      if (imp->OuterIndex < 0)
      {
        FObjectImport* outer = GetImportObject(imp->OuterIndex);
        outer->Inner.push_back(imp);
      }
    }
    else
    {
      RootImports.push_back(imp);
    }
  }

#if _DEBUG
  for (FObjectExport* exp : Exports)
  {
    if (exp->ExportFlags & EF_ScriptPatcherExport)
    {
      // If here: probably need to apply stack frames, patch the export table and fix net count
      DBreak();
    }
  }
#endif

#if DUMP_PACKAGES
  _DebugDump();
#endif

  // Not bulletproof. Will work only for exposed packages
  if (CompositPackageList.count(GetPackageName().ToUpper()))
  {
    Composite = true;
  }

  Ready.store(true);
  Loading.store(false);
}

bool FPackage::Save(PackageSaveContext& context)
{
  if (context.EmbedObjectPath && IsComposite() && GetFolderName() == NAME_None)
  {
    const FString packageName = GetPackageName();
    const auto& map = FPackage::GetCompositePackageMap();
    if (map.count(packageName))
    {
      const FCompositePackageMapEntry& entry = map.at(packageName);
      SetFolderName("MOD:" + entry.ObjectPath);
    }
  }

  if (context.DisableTextureCaching)
  {
    if (Summary.TextureAllocations.TextureTypes.size())
    {
      Summary.TextureAllocations.TextureTypes.clear();
      MarkDirty();
    }
    for (FObjectExport* exp : Exports)
    {
      if (exp->GetClassName() == UTexture2D::StaticClassName())
      {
        ((UTexture2D*)GetObject(exp, true))->DisableCaching();
      }
    }
  }
  
  if (!IsDirty())
  {
    if (context.Compression == COMPRESS_None)
    {
      if (context.ProgressDescriptionCallback)
      {
        context.ProgressDescriptionCallback("Saving...");
      }
      
      FReadStream readStream(Summary.DataPath);
      FILE_OFFSET size = readStream.GetSize();

      FPackageSummary summary;
      readStream << summary;
      if (summary.CompressionFlags != context.Compression && summary.CompressedChunks.size() && summary.PackageFlags & PKG_StoreCompressed)
      {
        FILE_OFFSET startOffset = INT_MAX;
        FILE_OFFSET totalDecompressedSize = 0;

        std::vector<FCompressedChunk> chunks = summary.CompressedChunks;
        void** compressedChunksData = new void* [chunks.size()];

        for (int32 idx = 0; idx < summary.CompressedChunks.size(); ++idx)
        {
          FCompressedChunk& chunk = chunks[idx];
          compressedChunksData[idx] = malloc(chunk.CompressedSize);
          readStream.SetPosition(chunk.CompressedOffset);
          readStream.SerializeBytes(compressedChunksData[idx], chunk.CompressedSize);
          totalDecompressedSize += chunk.DecompressedSize;
          if (chunk.DecompressedOffset < startOffset)
          {
            startOffset = chunk.DecompressedOffset;
          }
        }
        
        summary.CompressedChunks.clear();
        summary.CompressionFlags = COMPRESS_None;
        summary.PackageFlags &= ~PKG_StoreCompressed;

        void* decompressedPackageData = malloc(readStream.GetPosition() + totalDecompressedSize);

        concurrency::parallel_for(size_t(0), size_t(chunks.size()), [&chunks, compressedChunksData, decompressedPackageData, startOffset](size_t idx) {
          const FCompressedChunk& chunk = chunks[idx];
          uint8* dst = (uint8*)decompressedPackageData + chunk.DecompressedOffset - startOffset;
          LZO::Decompress(compressedChunksData[idx], chunk.CompressedSize, dst, chunk.DecompressedSize);
        });

        MWrightStream memStream(decompressedPackageData, readStream.GetPosition() + totalDecompressedSize);
        memStream << summary;

        FWriteStream writeStream(context.Path);
        writeStream.SerializeBytes(memStream.GetAllocation(), memStream.GetSize());

        free(decompressedPackageData);
        return true;
      }

      // Package is already decompressed. Copy the data.

      void* tmp = malloc(size);
      readStream.SetPosition(0);
      readStream.SerializeBytes(tmp, size);
      if (!readStream.IsGood() || !size)
      {
        context.Error = "Failed to read source package.";
        free(tmp);
        return false;
      }
      if (context.ProgressCallback)
      {
        context.ProgressCallback(50);
      }
      if (context.ProgressDescriptionCallback)
      {
        context.ProgressDescriptionCallback("Writing data...");
      }
      FWriteStream writeStream(context.Path);
      writeStream.SerializeBytes(tmp, size);
      if (!writeStream.IsGood())
      {
        context.Error = "Failed to write the data.";
        free(tmp);
        return false;
      }
      free(tmp);
      return true;
    }

    // Compress the package without fancy object serialization

    FReadStream readStream(Summary.DataPath);
    if (!readStream.IsGood() || !readStream.GetSize())
    {
      context.Error = "Failed to read source package.";
      return false;
    }

    FPackageSummary summary;
    readStream << summary;
    FILE_OFFSET dataStart = readStream.GetPosition();
    FILE_OFFSET size = readStream.GetSize() - dataStart;

    // Copy-paste data if the package is already compressed
    if (summary.CompressedChunks.size() && summary.CompressionFlags == context.Compression)
    {
      summary.PackageFlags |= PKG_StoreCompressed;

      FWriteStream writeStream(context.Path);
      writeStream << summary;

      void* pkgData = malloc(size);
      readStream.SerializeBytes(pkgData, size);
      writeStream.SerializeBytes(pkgData, size);
      free(pkgData);

      if (!writeStream.IsGood() || !readStream.IsGood())
      {
        context.Error = "IO error. Check source and destination are available for read and write!";
        return false;
      }
      return true;
    }

    void* uncompressedData = malloc(size);
    readStream.SerializeBytes(uncompressedData, size);

    int32	totalChunkCount = (size + COMPRESSED_BLOCK_SIZE - 1) / COMPRESSED_BLOCK_SIZE;
    summary.CompressedChunks.resize(totalChunkCount);
    summary.PackageFlags |= PKG_StoreCompressed;
    summary.CompressionFlags = context.Compression;

    FWriteStream writeStream(context.Path);
    writeStream << summary;

    const int32 compressedDataSize = 2 * COMPRESSED_BLOCK_SIZE;
    void* compressedData = malloc(compressedDataSize);

    int32 chunkIndex = 0;
    int32 remainingSize = size;
    FILE_OFFSET offset = dataStart;

    while (remainingSize > 0)
    {
      int32 sizeToCompress = std::min(remainingSize, COMPRESSED_BLOCK_SIZE);
      int32 compressedSize = compressedDataSize;

      if (!CompressMemory(COMPRESS_LZO, compressedData, &compressedSize, (uint8*)uncompressedData + offset, sizeToCompress))
      {
        context.Error = "Failed to compress data.";
        free(compressedData);
        free(uncompressedData);
        return false;
      }

      summary.CompressedChunks[chunkIndex].DecompressedSize = sizeToCompress;
      summary.CompressedChunks[chunkIndex].CompressedOffset = writeStream.GetPosition();
      summary.CompressedChunks[chunkIndex].DecompressedOffset = offset;
      offset += sizeToCompress;

      writeStream.SerializeBytes(compressedData, compressedSize);

      summary.CompressedChunks[chunkIndex].CompressedSize = compressedSize;
      chunkIndex++;
      remainingSize -= COMPRESSED_BLOCK_SIZE;
    }

    writeStream.SetPosition(0);
    writeStream << summary;

    free(compressedData);
    free(uncompressedData);

    if (!writeStream.IsGood() || !readStream.IsGood())
    {
      context.Error = "IO error. Check source and destination are available for read and write!";
      return false;
    }
    return true;
  }

  FWriteStream writer(context.Path);
  if (!writer.IsGood())
  {
    context.Error = "Failed to write data!";
    return false;
  }
  writer.SetPackage(this);
  
  // Stream of the decompressed temporary source.
  FReadStream reader(Summary.DataPath); // TODO: we may have no data path for a new packages.
  reader.SetPackage(this);
  if (!reader.IsGood())
  {
    context.Error = "Failed to read the source package!";
    return false;
  }

  // Get the original package summary
  FPackageSummary summary; // TODO: we may not have source package(e.g. a new packge)
  reader << summary;

  if (!Summary.ImportGuidsCount && !Summary.ExportGuidsCount)
  {
    // Non-zero values force linker to seek for Guid map.
    // Set offset to INDEX_NONE to make games linker ignore the map.
    Summary.ImportExportGuidsOffset = INDEX_NONE;
  }
  else
  {
    // Never seen these maps in Tera. Need to implement if BHS used them somewhere
    DBreak();
    context.Error = "Packages with crosslevel guids are not supported yet!";
    return false;
  }
  
  Summary.ThumbnailTableOffset = 0; // Cleanup after BHS. They forgot to strip these.

  Summary.NamesCount = (uint32)Names.size();
  Summary.ExportsCount = (uint32)Exports.size();
  Summary.ImportsCount = (uint32)Imports.size();

  Summary.CompressionFlags = context.Compression;
  if (context.Compression == COMPRESS_None)
  {
    Summary.PackageFlags &= ~PKG_StoreCompressed;
  }
  else
  {
    Summary.PackageFlags |= PKG_StoreCompressed;
  }

  Summary.PackageFlags &= ~PKG_Dirty;

  writer << Summary;

  std::vector<FObjectExport*> sortedExports = Exports;
  std::sort(sortedExports.begin(), sortedExports.end(), [](FObjectExport* a, FObjectExport* b) {
    return a->SerialOffset < b->SerialOffset;
  });

  // Check if header size did change and we honor offsets
  bool moveTables = context.PreserveOffsets && (Summary.NamesOffset != writer.GetPosition());

  if (!moveTables && context.PreserveOffsets)
  {
    // TODO: exports have non-const size. Table can become larger even if elements count is lower. Fixed-size calculation
    if (Summary.ExportsCount > summary.ExportsCount || Summary.ImportsCount > summary.ImportsCount)
    {
      moveTables = true;
    }
  }

  Summary.NamesOffset = writer.GetPosition();
  for (FNameEntry& e : Names)
  {
    writer << e;
  }

  if (context.PreserveOffsets)
  {
    if (!moveTables && writer.GetPosition() - Summary.NamesOffset > Summary.NamesSize)
    {
      // Names table become larger. We need to move tables to preserve offsets
      moveTables = true;
    }
    if (!moveTables)
    {
      // Check if the tables were moved during previous save
      for (FObjectExport* exp : Exports)
      {
        if (exp->SerialOffset < Summary.ImportsOffset)
        {
          moveTables = true;
          break;
        }
      }
    }
  }

  if (moveTables)
  {
    // Prevent the game from buffering whole package if we are moving tables to the end.
    Summary.HeaderSize = writer.GetPosition();
  }
  else
  {
    Summary.ImportsOffset = writer.GetPosition();
    for (FObjectImport* imp : Imports)
    {
      writer << *imp;
    }

    Summary.ExportsOffset = writer.GetPosition();
    for (FObjectExport* exp : Exports)
    {
      writer << *exp;
    }

    Summary.DependsOffset = writer.GetPosition();
    for (auto& dep : Depends)
    {
      writer << dep;
    }
    Summary.HeaderSize = writer.GetPosition();
  }

  FILE_OFFSET exportsStart = sortedExports.front()->SerialOffset;
  auto appendZeroed = [](FStream& s, FILE_OFFSET size) {
    void* tmp = calloc(size, 1);
    s.SerializeBytes(tmp, size);
    free(tmp);
  };

  if (exportsStart > writer.GetPosition())
  {
    appendZeroed(writer, exportsStart - writer.GetPosition());
  }
  else if (exportsStart < writer.GetPosition())
  {
    DBreakIf(context.PreserveOffsets);
    context.Error = "Package has too much changes in the header. Can't save it with the 'Preserve Offsets' flag";
    return false;
  }

  if (context.MaxProgressCallback)
  {
    context.MaxProgressCallback(Exports.size());
  }
  if (context.ProgressCallback)
  {
    context.ProgressCallback(0);
  }
  if (context.ProgressDescriptionCallback)
  {
    context.ProgressDescriptionCallback("Serializing objects...");
  }

  // List of holes left after moving dirty objects. Offset, Size.
  std::vector<std::pair<FILE_OFFSET, FILE_OFFSET>> holes;
  int32 idx = 0;
  for (FObjectExport* exp : sortedExports)
  {
    if (exp->ObjectFlags & RF_Marked)
    {
      if (context.PreserveOffsets)
      {
        holes.push_back({ writer.GetPosition(), exp->SerialSize });
        appendZeroed(writer, exp->SerialSize);
      }
      else
      {
        exp->SerialOffset = writer.GetPosition();
        UObject* obj = ExportObjects[exp->ObjectIndex];
        obj->Serialize(writer);
        exp->SerialSize = writer.GetPosition() - exp->SerialOffset;
      }
    }
    else
    {
      if (context.PreserveOffsets && writer.GetPosition() != exp->SerialOffset)
      {
        if (writer.GetPosition() < exp->SerialOffset)
        {
          holes.push_back({ writer.GetPosition(), exp->SerialOffset - writer.GetPosition() });
          appendZeroed(writer, exp->SerialOffset - writer.GetPosition());
        }
        else
        {
          DBreak();
          LogE("Failed to preserve offset of %s", exp->GetObjectName().C_str());
        }
      }
      if (!context.FullRecook)
      {
        reader.SetPosition(exp->SerialOffset);
        void* data = malloc(exp->SerialSize);
        reader.SerializeBytes(data, exp->SerialSize);
        exp->SerialOffset = writer.GetPosition();
        writer.SerializeBytes(data, exp->SerialSize);
        free(data);
      }
      else
      {
        exp->SerialOffset = writer.GetPosition();
        UObject* obj = ExportObjects[exp->ObjectIndex];
        if (!obj->IsLoaded())
        {
          obj->Load();
        }
        obj->Serialize(writer);
        exp->SerialSize = writer.GetPosition() - exp->SerialOffset;
      }
    }
    if (context.ProgressCallback)
    {
      context.ProgressCallback(++idx);
    }
  }

  if (moveTables)
  {
    Summary.ImportsOffset = writer.GetPosition();
    for (FObjectImport* imp : Imports)
    {
      writer << *imp;
    }

    Summary.ExportsOffset = writer.GetPosition();
    for (FObjectExport* exp : Exports)
    {
      writer << *exp;
    }

    Summary.DependsOffset = writer.GetPosition();
    for (auto& dep : Depends)
    {
      writer << dep;
    }
  }

  if (context.PreserveOffsets)
  {
    for (FObjectExport* exp : sortedExports)
    {
      if (exp->ObjectFlags & RF_Marked)
      {
        MWrightStream tmpWriter(nullptr, 1024 * 1024, writer.GetPosition());
        tmpWriter.SetPackage(this);

        exp->SerialOffset = writer.GetPosition();
        UObject* obj = ExportObjects[exp->ObjectIndex];
        DBreakIf(!obj->IsLoaded());
        obj->Serialize(tmpWriter);
        exp->SerialSize = tmpWriter.GetPosition() - exp->SerialOffset;

        // Try to fit data in an existing hole
        int32 foundHole = -1;
        FILE_OFFSET holeSizeDelta = INT32_MAX;
        for (int32 idx = 0; idx < holes.size(); ++idx)
        {
          FILE_OFFSET holeSize = holes[idx].second;
          if (holeSize >= exp->SerialSize && holeSize - exp->SerialSize < holeSizeDelta)
          {
            holeSizeDelta = holeSize - exp->SerialSize;
            foundHole = idx;
          }
        }

        if (foundHole >= 0)
        {
          auto& hole = holes[foundHole];
          exp->SerialOffset = hole.first;
          
          FILE_OFFSET tmpPos = writer.GetPosition();

          writer.SetPosition(exp->SerialOffset);
          obj->Serialize(writer);
         
          if (exp->SerialSize != writer.GetPosition() - exp->SerialOffset)
          {
            context.Error = "Failed to save package due to ambiguos object size!";
            LogE("Failed to save package due to ambiguos object size!");
            return false;
          }
          
          hole.first += exp->SerialSize;
          hole.second -= exp->SerialSize;
          DBreakIf(hole.second < 0);

          writer.SetPosition(tmpPos);
        }
        else
        {
          writer.SerializeBytes(tmpWriter.GetAllocation(), tmpWriter.GetSize() - tmpWriter.GetOffset());
        }
      }
    }
  }

  // Apply header changes
  writer.SetPosition(0);
  writer << Summary;

  // Unmark objects and apply exports table changes 
  std::vector<FObjectExport*> marked;
  writer.SetPosition(Summary.ExportsOffset);
  for (FObjectExport* exp : Exports)
  {
    if (exp->ObjectFlags & RF_Marked)
    {
      marked.push_back(exp);
      exp->ObjectFlags &= ~RF_Marked;
    }
    writer << *exp;
  }
  // Since this is a "Save As" we need to restore dirty flags
  for (FObjectExport* exp : marked)
  {
    exp->ObjectFlags |= RF_Marked;
  }
  MarkDirty();
  
  bool isOk = writer.IsGood();
  if (!isOk)
  {
    context.Error = "Unknown error! Failed to write data!";
  }
  return isOk;
}

FString FPackage::GetCompositePath()
{
  if (IsComposite() && CompositPackageMap.count(GetPackageName()))
  {
    return CompositPackageMap[GetPackageName()].ObjectPath;
  }
  return FString();
}

VObjectExport* FPackage::CreateVirtualExport(const char* objName, const char* clsName)
{
  VObjectExport* exp = new VObjectExport(this, objName, clsName);
  VExports.push_back(exp);
  return exp;
}

FString FPackage::GetPackageName(bool extension) const
{
  if (extension)
  {
    return Summary.PackageName;
  }
  else
  {
    FString name = Summary.PackageName;
    if (name.Length())
    {
      size_t start = 0;
      size_t idx = name.FindFirstOf('.', start);
      while (idx == 0)
      {
        start++;
        idx = name.FindFirstOf('.', start);
      }
      if (idx != std::string::npos)
      {
        name = name.Substr(start, idx);
      }
    }
    return name;
  }
}

UObject* FPackage::GetObject(PACKAGE_INDEX index, bool load)
{
  if (index > 0)
  {
    return GetObject(GetExportObject(index), load);
  }
  else if (index < 0)
  {
    return GetObject(GetImportObject(index), load);
  }
  return nullptr;
}

UObject* FPackage::GetObject(FObjectImport* imp, bool load)
{
  if (imp->Package == this)
  {
    if (UObject* obj = GetCachedImportObject(imp->ObjectIndex))
    {
      if (load)
      {
        obj->Load();
      }
      return obj;
    }
    FString impPkgName = imp->GetPackageName();

    if (impPkgName != GetPackageName(false))
    {
      // Check already loaded packages
      {
        std::scoped_lock<std::mutex> lock(ExternalPackagesMutex);
        for (std::shared_ptr<FPackage>& external : ExternalPackages)
        {
          if (impPkgName == external->GetPackageName(false))
          {
            UObject* obj = external->GetObject(imp, load);
            ImportObjects[imp->ObjectIndex] = obj;
            return obj;
          }
        }
      }
      // Load external package
      if (auto package = GetPackageNamed(impPkgName))
      {
        package->Load();
        if (UObject* obj = package->GetObject(imp, load))
        {
          std::scoped_lock<std::mutex> lock(ExternalPackagesMutex);
          ExternalPackages.emplace_back(package);
          ImportObjects[imp->ObjectIndex] = obj;
          return obj;
        }
        UnloadPackage(package);
      }
      LogW("%s: Couldn't find import %s(%s)", GetPackageName().C_str(), imp->GetObjectName().C_str(), imp->GetClassName().C_str());
      return nullptr;
    }
  }
  for (VObjectExport* vexp : VExports)
  {
    if (vexp->GetObjectName() == imp->GetObjectName() && vexp->GetClassName() == imp->GetClassName())
    {
      return vexp->GetObject();
    }
  }
  std::vector<FObjectExport*> exps = GetExportObject(imp->GetObjectName());
  FString className = imp->GetClassName();
  for (FObjectExport* exp : exps)
  {
    if (exp->GetClassName() == className)
    {
      return GetObject(exp);
    }
  }
  return nullptr;
}

UObject* FPackage::GetObject(FObjectExport* exp, bool load)
{
  if (exp->ObjectIndex == VEXP_INDEX)
  {
    for (VObjectExport* vexp : VExports)
    {
      if (vexp == exp)
      {
        return vexp->GetObject();
      }
    }
  }

  UObject* obj = GetCachedExportObject(exp->ObjectIndex);
  if (obj && load)
  {
    obj->Load();
  }
  if (obj && exp->ExportFlags & EF_ForcedExport && AllowForcedExportResolving)
  {
    // Don't load packages or inner objects
    if (obj->GetClassName() != NAME_Package && obj->GetOuter() && obj->GetOuter()->GetClassName() == NAME_Package)
    {
      if (UObject* ext = GetCachedForcedObject(exp->ObjectIndex))
      {
        if (load)
        {
          ext->Load();
        }
        return ext;
      }
      else if (ext = SetCachedForcedObject(exp->ObjectIndex, GetForcedExport(exp)))
      {
        if (load)
        {
          ext->Load();
        }
        return ext;
      }
      else
      {
        LogW("Failed to find external: \"%s\"", obj->GetFullObjectName().C_str());
      }
    }
  }
  return obj;
}

UObject* FPackage::GetForcedExport(FObjectExport* exp)
{
  if (!AllowForcedExportResolving)
  {
    return nullptr;
  }
  FObjectExport* outer = nullptr;
  for (outer = exp->Outer; outer && outer->Outer; outer = outer->Outer);
  if (!outer)
  {
    return nullptr;
  }
  FString outerName = outer->GetObjectName();
  {
    std::scoped_lock<std::mutex> l(MissingPackagesMutex);
    if (std::find(MissingPackages.begin(), MissingPackages.end(), outerName) != MissingPackages.end())
    {
      return nullptr;
    }
  }

  UObject* incomplete = GetCachedExportObject(exp->ObjectIndex);
  incomplete->Load();

  UObject* result = nullptr;
  {
    std::scoped_lock<std::mutex> l(ExternalPackagesMutex);
    for (auto& p : ExternalPackages)
    {
      if (outer->PackageGuid.IsValid())
      {
        if (outer->PackageGuid == p->GetGuid())
        {
          result = p->GetObject(incomplete->GetNetIndex(), incomplete->GetObjectName(), incomplete->GetClassName());
          break;
        }
      }
      else
      {
        FString fname = p->GetPackageName().Filename(false);
        if (fname.StartWith(outerName))
        {
          if (UObject* object = p->GetObject(incomplete->GetNetIndex(), incomplete->GetObjectName(), incomplete->GetClassName()))
          {
            result = object;
            break;
          }
        }
      }
    }
  }
  if (result)
  {
    return result;
  }

  bool packageNotFound = true;
  std::vector<FString> incompleteMatch;
  for (const FString& path : DirCache)
  {
    if (path.Filename(false).StartWith(outerName))
    {
      packageNotFound = false;
      std::shared_ptr<FPackage> p = nullptr;
      FString completePath(RootDir);
      completePath = completePath.FStringByAppendingPath(path);
      if ((p = FPackage::GetPackage(completePath)))
      {
        if (!outer->PackageGuid.IsValid() || outer->PackageGuid == p->GetGuid())
        {
          p->Load();
          if (UObject* object = p->GetObject(incomplete->GetNetIndex(), incomplete->GetObjectName(), incomplete->GetClassName()))
          {
            {
              std::scoped_lock<std::mutex> l(ExternalPackagesMutex);
              ExternalPackages.push_back(p);
            }
            return object;
          }
        }
        else
        {
          incompleteMatch.push_back(completePath);
        }
      }
      FPackage::UnloadPackage(p);
    }
  }

  // We failed to find the package with matching GUIDs. Lets check packages with the same name but ignor GUID this time
  for (const FString& completePath : incompleteMatch)
  {
    std::shared_ptr<FPackage> p = nullptr;
    if ((p = FPackage::GetPackage(completePath)))
    {
      p->Load();
      if (UObject* object = p->GetObject(incomplete->GetNetIndex(), incomplete->GetObjectName(), incomplete->GetClassName()))
      {
        {
          std::scoped_lock<std::mutex> l(ExternalPackagesMutex);
          ExternalPackages.push_back(p);
        }
        return object;
      }
    }
    FPackage::UnloadPackage(p);
  }

  if (packageNotFound)
  {
    std::scoped_lock<std::mutex> l(MissingPackagesMutex);
    MissingPackages.push_back(outer->GetObjectName());
  }

  return nullptr;
}

PACKAGE_INDEX FPackage::GetObjectIndex(UObject* object) const
{
  if (!object)
  {
    return 0;
  }
  if (object->GetPackage() != this)
  {
    std::scoped_lock<std::mutex> l(ImportObjectsMutex);
    for (const auto& p : ImportObjects)
    {
      if (p.second == object)
      {
        return p.first;
      }
    }
    UThrow("%s does not have import object for %s", GetPackageName().C_str(), object->GetObjectName().String().c_str());
  }
  return object->GetExportObject()->ObjectIndex;
}

PACKAGE_INDEX FPackage::GetNameIndex(const FString& name, bool insert)
{
  for (PACKAGE_INDEX index = 0; index < Names.size(); ++index)
  {
    if (Names[index].GetString() == name)
    {
      return index;
    }
  }
  if (insert)
  {
    Names.push_back(FNameEntry(name));
    return (PACKAGE_INDEX)Names.size() - 1;
  }
  return INDEX_NONE;
}

UClass* FPackage::LoadClass(PACKAGE_INDEX index)
{
  if (!index)
  {
    return nullptr;
  }

  // Speedup class load during class packages load
  if (index > 0)
  {
    UObject* obj = GetObject(index);
    FString objName = obj->GetObjectName();
    {
      std::scoped_lock<std::mutex> l(ClassMapMutex);
      if (!ClassMap.count(objName))
      {
        ClassMap[objName] = obj;
      }
    }
    return (UClass*)obj;
  }

  if (UObject* obj = GetCachedImportObject(index))
  {
    return (UClass*)obj;
  }

  FObjectImport* imp = GetImportObject(index);
  FString name = imp->GetObjectName();
  {
    std::scoped_lock<std::mutex> l(ClassMapMutex);
    if (ClassMap.count(name))
    {
      UObject* obj = ClassMap[name];
      SetCachedImportObject(index, obj);
      return (UClass*)obj;
    }
  }
  FString pkgName = imp->GetPackageName();
  if (pkgName == GetPackageName())
  {
    UObject* obj = GetObject(imp);
    if (obj)
    {
      std::scoped_lock<std::mutex> l(ClassMapMutex);
      ClassMap[name] = obj;
      return (UClass*)obj;
    }
  }
  {
    std::scoped_lock<std::mutex> l(ExternalPackagesMutex);
    for (auto pkg : ExternalPackages)
    {
      if (pkg->GetPackageName() == pkgName)
      {
        UObject* obj = pkg->GetObject(imp);
        if (obj)
        {
          std::scoped_lock<std::mutex> l(ClassMapMutex);
          ClassMap[name] = obj;
          ExternalPackages.push_back(pkg);
          return (UClass*)obj;
        }
        break;
      }
    }
  }
  if (auto pkg = GetPackageNamed(pkgName))
  {
    UObject* obj = pkg->GetObject(imp);
    if (obj)
    {
      std::scoped_lock<std::mutex> l(ClassMapMutex);
      ClassMap[name] = obj;
      std::scoped_lock<std::mutex> lock(ExternalPackagesMutex);
      ExternalPackages.push_back(pkg);
      return (UClass*)obj;
    }
    FPackage::UnloadPackage(pkg);
  }
  
  if (!MissingClasses.count(name))
  {
    LogE("Failed to load class %s", name.C_str());
    MissingClasses.insert(name);
  }
  return nullptr;
}

UClass* FPackage::GetClass(const FString& className)
{
  if (ClassMap.count(className))
  {
    return Cast<UClass>(ClassMap[className]);
  }
  return nullptr;
}

PACKAGE_INDEX FPackage::ImportClass(UClass* cls)
{
  if (cls->GetPackage() == this)
  {
    return cls->GetExportObject()->ObjectIndex;
  }

  FObjectImport* classPackageImport = nullptr;
  const FString classPackage = cls->GetPackage()->GetPackageName();
  for (FObjectImport* imp : Imports)
  {
    if (imp->GetObjectName() == cls->GetObjectName())
    {
      int x = 1;
    }
    if (imp->GetClassName() == UClass::StaticClassName() && imp->GetObjectName() == cls->GetObjectName())
    {
      return imp->ObjectIndex;
    }
    if (imp->GetClassName() == NAME_Package && imp->GetObjectName() == classPackage)
    {
      classPackageImport = imp;
    }
  }

  if (!classPackageImport)
  {
    classPackageImport = FObjectImport::CreateImport(this, classPackage, (UClass*)ClassMap[NAME_Package]);
    if (!classPackageImport)
    {
      return 0;
    }
    classPackageImport->ObjectIndex = -PACKAGE_INDEX(Imports.size()) - 1;
    Imports.push_back(classPackageImport);
    MarkDirty();
  }

  FObjectImport* classImport = FObjectImport::CreateImport(this, cls->GetObjectName(), nullptr);
  if (classImport)
  {
    classImport->ObjectIndex = -PACKAGE_INDEX(Imports.size()) - 1;
    Imports.push_back(classImport);
    MarkDirty();
    return classImport->ObjectIndex;
  }
  return 0;
}

void FPackage::AddNetObject(UObject* object)
{
  NetIndexMap[object->GetNetIndex()] = object;
}

UObject* FPackage::GetObject(NET_INDEX netIndex, const FString& name, const FString& className)
{
  UObject* result = nullptr;
  if (netIndex != INDEX_NONE && NetIndexMap.count(netIndex))
  {
    result = NetIndexMap[netIndex];
  }
  else
  {
    std::vector<FObjectExport*> exps = GetExportObject(name);
    UObject* obj = nullptr;
    for (FObjectExport* exp : exps)
    {
      if (exp->GetClassName() == className)
      {
        if (netIndex != INDEX_NONE)
        {
          obj = GetObject(exp);
          if (obj->GetNetIndex() == netIndex)
          {
            result = obj;
            break;
          }
        }
        else if (obj = GetObject(exp))
        {
          result = obj;
          break;
        }
      }
    }
  }
  return result;
}

std::vector<FObjectExport*> FPackage::GetExportObject(const FString& name)
{
  if (ObjectNameToExportMap.count(name))
  {
    return ObjectNameToExportMap[name];
  }
  for (VObjectExport* vexp : VExports)
  {
    if (vexp->GetObjectName() == name)
    {
      return { vexp };
    }
  }
  return std::vector<FObjectExport*>();
}

void FPackage::MarkDirty(bool dirty)
{
  if (dirty)
  {
    Summary.PackageFlags |= PKG_Dirty;
  }
  else
  {
    Summary.PackageFlags &= ~PKG_Dirty;
  }
}

FObjectImport* FPackage::GetImportObject(const FString& objectName, const FString& className) const
{
  for (FObjectImport* imp : Imports)
  {
    if (imp->GetObjectName() == objectName && imp->GetClassName() == className)
    {
      return imp;
    }
  }
  return nullptr;
}

bool FPackage::AddImport(UObject* object, FObjectImport*& output)
{
  if (!object || !object->GetPackage() || object->GetPackage() == this)
  {
    output = nullptr;
    return false;
  }

  FObjectImport* outerPackage = nullptr;
  FString outerPackageName = object->GetPackage()->GetPackageName();
  for (FObjectImport* imp : Imports)
  {
    if (imp->GetObjectName() == object->GetObjectName() &&
      imp->GetClassName() == object->GetClassName() &&
      imp->GetObjectPath() == object->GetObjectPath())
    {
      output = imp;
      return false;
    }
    if (imp->GetObjectName() == outerPackageName && imp->GetClassName() == NAME_Package)
    {
      outerPackage = imp;
    }
  }

  std::vector<UObject*> pathChain;
  {
    UObject* outer = object->GetOuter();
    while (outer)
    {
      pathChain.insert(pathChain.begin(), outer);
      outer = outer->GetOuter();
    }
  }

  if (!outerPackage)
  {
    FObjectImport* importObject = new FObjectImport(this, outerPackageName);
    importObject->ClassName.SetPackage(this);
    importObject->ClassName.SetString(NAME_Package);
    importObject->ClassPackage.SetPackage(this);
    importObject->ClassPackage.SetString("Core");
    Imports.push_back(importObject);
    outerPackage = importObject;
  }

  FObjectImport* outerImp = outerPackage;
  for (UObject* obj : pathChain)
  {
    obj->Load();
    if (!obj->GetClass())
    {
      LogE("Can't import %s! Outer object %s has no UClass!", object->GetObjectName().UTF8().c_str(), obj->GetObjectName().UTF8().c_str());
      output = nullptr;
      return false;
    }

    FObjectImport* imp = nullptr;
    bool added = AddImport(obj, imp);
    if (!imp)
    {
      output = nullptr;
      return false;
    }

    if (added && outerImp)
    {
      outerImp->Inner.push_back(imp);
      imp->OuterIndex = outerImp->ObjectIndex;
    }
    outerImp = imp;
  }

  FObjectImport* importObject = new FObjectImport(this, object->GetObjectName());
  importObject->ClassName.SetPackage(this);
  importObject->ClassName.SetString(object->GetClassName());
  importObject->ClassPackage.SetPackage(this);
  importObject->ClassPackage.SetString(object->GetClass() ? object->GetClass()->GetPackage()->GetPackageName() : "Core");

  if (outerImp)
  {
    outerImp->Inner.push_back(importObject);
    importObject->OuterIndex = outerImp->ObjectIndex;
  }

  Imports.push_back(importObject);
  importObject->ObjectIndex = -(PACKAGE_INDEX)Imports.size();
  output = importObject;
  ImportObjects[importObject->ObjectIndex] = object;
  return true;
}

void FPackage::RetainPackage(std::shared_ptr<FPackage> package)
{
  std::scoped_lock<std::mutex> l(ExternalPackagesMutex);
  ExternalPackages.push_back(package);
}

void FPackage::_DebugDump() const
{
#if DUMP_PACKAGES
  std::filesystem::path path = std::filesystem::path(DUMP_PATH) / GetPackageName().String();
  std::filesystem::create_directories(path);
  path /= "Info.txt";
  std::ofstream ds(path.wstring());
  ds << "SourcePath: \"" << Summary.SourcePath.UTF8() << "\"\n";
  ds << "DataPath: \"" << Summary.DataPath.UTF8() << "\"\n";
  if (GetFileVersion() > VER_TERA_CLASSIC && CompositeDataPath.Size())
  {
    ds << "CompositeSourcePath: \"" << CompositeSourcePath.UTF8() << "\"\n";
    ds << "CompositeDataPath: \"" << CompositeDataPath.UTF8() << "\"\n";
  }
  ds << "Version: " << Summary.FileVersion << "/" << Summary.LicenseeVersion << std::endl;
  ds << "HeaderSize: " << Summary.HeaderSize << std::endl;
  ds << "FolderName: \"" << Summary.FolderName.UTF8() << "\"\n";
  ds << "PackageFlags: " << PackageFlagsToString(Summary.PackageFlags).UTF8() << std::endl;
  ds << "CompressionFlags: " << Sprintf("0x%08X", Summary.CompressionFlags) << std::endl;
  ds << "PackageSource: " << Sprintf("0x%08X", Summary.PackageSource) << std::endl;
  ds << "Guid: " << Summary.Guid.String().UTF8() << std::endl;
  ds << "EngineVersion: " << Summary.EngineVersion << " ContentVersion: " << Summary.ContentVersion << std::endl;
  ds << "Names Count: " << Summary.NamesCount << " Offset: " << Summary.NamesOffset << std::endl;
  ds << "Exports Count: " << Summary.ExportsCount << " Offset: " << Summary.ExportsOffset << std::endl;
  ds << "Imports Count: " << Summary.ImportsCount << " Offset: " << Summary.ImportsOffset << std::endl;
  ds << "Depends Count: " << Summary.ExportsCount << " Offset: " << Summary.DependsOffset << std::endl;
  if (GetFileVersion() > VER_TERA_CLASSIC)
  {
    ds << "ImportGuids Count: " << Summary.ImportGuidsCount << " ExportGuids Count: " << Summary.ExportGuidsCount << " Offset: " << Summary.ImportExportGuidsOffset << std::endl;
    ds << "ThumbnailTable Offset: " << Summary.ThumbnailTableOffset << std::endl;
  }
  ds << "Generations:\n";
  for (const FGeneration& generation : Summary.Generations)
  {
    ds << "\tExports: " << generation.Exports << " Names: " << generation.Names << " Nets: " << generation.NetObjects << std::endl;
  }
  if (Summary.TextureAllocations.TextureTypes.size() && GetFileVersion() > VER_TERA_CLASSIC)
  {
    ds << "TextureAllocations:\n";
    for (const FTextureAllocations::FTextureType& ttype : Summary.TextureAllocations.TextureTypes)
    {
      ds << "\tSizeX: " << ttype.SizeX << " SizeY: " << ttype.SizeY << " NumMips: " << ttype.NumMips;
      ds << " Format: " << PixelFormatToString(ttype.Format).UTF8() << " TexFlags: " << Sprintf("0x%08X", ttype.TexCreateFlags);
      ds << " ExportIndices: ";
      for (const int32& idx : ttype.ExportIndices)
      {
        ds << idx << ", ";
      }
      ds << std::endl;
    }
  }
  if (Summary.AdditionalPackagesToCook.size())
  {
    ds << "AdditionalPackagesToCook:\n";
    for (const FString& pkg : Summary.AdditionalPackagesToCook)
    {
      ds << "\t" << pkg.UTF8() << std::endl;
    }
  }

  ds << "Names:\n";
  for (int32 index = 0; index < Names.size(); ++index)
  {
    ds << "\t[" << index << "]" << Names[index].GetString().UTF8() << "\n";
  }
  ds << "\n\nExports:\n";

  std::function<void(FObjectExport*, int32)> printExp;
  printExp = [&ds, &printExp](FObjectExport* exp, int32 depth) {
    for (int32 i = 0; i < depth; ++i)
    {
      ds << "\t";
    }
    ds << "[" << exp->ObjectIndex << "]" << exp->GetFullObjectName().UTF8();
    ds << "(Offset: " << exp->SerialOffset << ", Size: " << exp->SerialSize;
    if (exp->ExportFlags)
    {
      ds << ", EF: " << ExportFlagsToString(exp->ExportFlags).UTF8();
    }
    ds << ", OF: " << ObjectFlagsToString(exp->ObjectFlags).UTF8();
    ds << ")\n";
    depth++;
    for (FObjectExport* cexp : exp->Inner)
    {
      printExp(cexp, depth);
    }
  };
  for (FObjectExport* e : RootExports)
  {
    printExp(e, 1);
  }
  ds << "\n\nImports:\n";
  std::function<void(FObjectImport*, int32)> printImp;
  printImp = [&ds, &printImp](FObjectImport* imp, int32 depth) {
    for (int32 i = 0; i < depth; ++i)
    {
      ds << "\t";
    }
    ds << "[" << imp->ObjectIndex << "]" << imp->GetFullObjectName().UTF8() << "\n";
    depth++;
    for (FObjectImport* cimp : imp->Inner)
    {
      printImp(cimp, depth);
    }
  };
  for (FObjectImport* i : RootImports)
  {
    printImp(i, 1);
  }
#endif
}

UObject* FPackage::GetCachedExportObject(PACKAGE_INDEX index) const
{
  std::scoped_lock<std::mutex> l(ExportObjectsMutex);
  if (ExportObjects.count(index))
  {
    return ExportObjects.at(index);
  }
  return nullptr;
}

UObject* FPackage::GetCachedForcedObject(PACKAGE_INDEX index) const
{
  std::scoped_lock<std::mutex> l(ForcedObjectsMutex);
  if (ForcedObjects.count(index))
  {
    return ForcedObjects.at(index);
  }
  return nullptr;
}

UObject* FPackage::GetCachedImportObject(PACKAGE_INDEX index) const
{
  std::scoped_lock<std::mutex> l(ImportObjectsMutex);
  if (ImportObjects.count(index))
  {
    return ImportObjects.at(index);
  }
  return nullptr;
}

UObject* FPackage::SetCachedExportObject(PACKAGE_INDEX index, UObject* obj)
{
  std::scoped_lock<std::mutex> l(ExportObjectsMutex);
  ExportObjects[index] = obj;
  return obj;
}

UObject* FPackage::SetCachedForcedObject(PACKAGE_INDEX index, UObject* obj)
{
  std::scoped_lock<std::mutex> l(ForcedObjectsMutex);
  ForcedObjects[index] = obj;
  return obj;
}

UObject* FPackage::SetCachedImportObject(PACKAGE_INDEX index, UObject* obj)
{
  std::scoped_lock<std::mutex> l(ImportObjectsMutex);
  ImportObjects[index] = obj;
  return obj;
}
