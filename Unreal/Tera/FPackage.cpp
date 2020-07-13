#include "FPackage.h"
#include "FStream.h"
#include "FObjectResource.h"
#include "UObject.h"
#include "UClass.h"

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <ppl.h>

std::string FPackage::RootDir;
std::vector<std::shared_ptr<FPackage>> FPackage::LoadedPackages;
std::vector<std::shared_ptr<FPackage>> FPackage::DefaultClassPackages;
std::vector<std::string> FPackage::DirCache;

namespace
{
  const char* PackageListName = "Packages.list";
  const std::vector<std::string> DefaultClassPackageNames = { "Core.u", "Engine.u", "S1Game.u" };

  void BuildPackageList(const std::string& path, std::vector<std::string>& dirCache)
  {
    DLog("Building package list: \"%s\"", path.c_str());
    std::filesystem::path fspath(A2W(path));
    dirCache.resize(0);
    std::vector<std::filesystem::path> tmpPaths;
    for (auto& p : std::filesystem::recursive_directory_iterator(fspath))
    {
      if (!p.is_regular_file() || !p.file_size())
      {
        continue;
      }
      std::filesystem::path itemPath = p.path();
      std::string ext = itemPath.extension().string();
      if (Stricmp(ext, ".gpk") || Stricmp(ext, ".gmp") || Stricmp(ext, ".upk") || Stricmp(ext, ".u"))
      {
        tmpPaths.push_back(itemPath);
      }
    }

    std::sort(tmpPaths.begin(), tmpPaths.end(), [](std::filesystem::path& a, std::filesystem::path& b) {
      auto tA = a.filename().replace_extension();
      auto tB = b.filename().replace_extension();
      return tA.wstring() < tB.wstring();
    });

    std::filesystem::path listPath = fspath / PackageListName;
    std::wofstream s(listPath.wstring());
    for (const auto& path : tmpPaths)
    {
      std::wstring wpath = path.wstring();
      dirCache.push_back(W2A(wpath));
      s << wpath << std::endl;
    }
    s.close();
    DLog("Found %ld items", dirCache.size());
  }
}


void FPackage::SetRootPath(const std::string& path)
{
  RootDir = path;
  std::filesystem::path listPath(A2W(path));
  listPath /= PackageListName;

  if (std::filesystem::exists(listPath))
  {
    std::wifstream s(listPath);
    if (s.is_open())
    {
      std::wstring l;
      while (std::getline(s, l))
      {
        if (l.size())
        {
          std::filesystem::path itemPath(l);
          std::string ext = itemPath.extension().string();
          if (Stricmp(ext, ".gpk") || Stricmp(ext, ".gmp") || Stricmp(ext, ".upk") || Stricmp(ext, ".u"))
          {
            DirCache.push_back(W2A(l));
          }
        }
      }
      s.close();
    }
    else
    {
      BuildPackageList(path, DirCache);
    }
  }
  else
  {
    BuildPackageList(path, DirCache);
  }
}

void FPackage::LoadDefaultClassPackages()
{
  for (const auto& name : DefaultClassPackageNames)
  {
    if (auto package = LoadPackageNamed(name))
    {
      DefaultClassPackages.push_back(package);
    }
  }
}

void FPackage::UnloadDefaultClassPackages()
{
  for (auto package : DefaultClassPackages)
  {
    UnloadPackage(package.get());
  }
  DefaultClassPackages.clear();
}

std::shared_ptr<FPackage> FPackage::LoadPackage(const std::string& path)
{
  std::shared_ptr<FPackage> found = nullptr;
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
  DLog("Loading package: %s", path.c_str());
  FStream *stream = new FReadStream(path);
  FPackageSummary sum;
  sum.SourcePath = path;
  sum.DataPath = path;
  sum.PackageName = W2A(std::filesystem::path(path).filename().wstring());
  (*stream) << sum;
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
    FStream* tempStream = new FWriteStream(sum.DataPath);

    std::vector<FCompressedChunk> tmpChunks;
    std::swap(sum.CompressedChunks, tmpChunks);
    (*tempStream) << sum;
    std::swap(sum.CompressedChunks, tmpChunks);

    uint8* decompressedData = (uint8*)malloc(totalDecompressedSize);
    concurrency::parallel_for(size_t(0), sum.CompressedChunks.size(), [sum, stream, compressedChunksData, decompressedData, startOffset](size_t idx) {
      const FCompressedChunk& chunk = sum.CompressedChunks[idx];
      uint8* dst = decompressedData + chunk.DecompressedOffset - startOffset;
      LZO::Decompress(compressedChunksData[idx], chunk.CompressedSize, dst, chunk.DecompressedSize);
      free(compressedChunksData[idx]);
    });
    delete[] compressedChunksData;
    tempStream->SerializeBytes(decompressedData, totalDecompressedSize);
    free(decompressedData);
    delete tempStream;
    delete stream;

    // Read decompressed header
    stream = new FReadStream(sum.DataPath);
    (*stream) << sum;
  }
  
  delete stream;
  std::shared_ptr<FPackage> package = LoadedPackages.emplace_back(new FPackage(sum));
  package->Initialize();
  return package;
}

std::shared_ptr<FPackage> FPackage::LoadPackageNamed(const std::string& name, FGuid guid)
{
  std::shared_ptr<FPackage> found = nullptr;
  for (auto package : LoadedPackages)
  {
    if (Wstricmp(package->GetPackageName(), name))
    {
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
    LoadedPackages.push_back(found);
    return found;
  }
  bool updatedDirCache = false;
  while (1)
  {
    std::wstring wname = A2W(name);
    for (std::string& path : DirCache)
    {
      std::wstring filename = std::filesystem::path(A2W(path)).filename().wstring();
      if (filename.size() < wname.size())
      {
        continue;
      }
      if (std::mismatch(wname.begin(), wname.end(), filename.begin()).first == wname.end())
      {
        if (auto package = LoadPackage(path))
        {
          if (!guid.IsValid() || guid == package->GetGuid())
          {
            return package;
          }
          UnloadPackage(package.get());
        }
      }
    }
    // Update DirCache if we couldn't find the package
    if (updatedDirCache)
    {
      break;
    }
    BuildPackageList(RootDir, DirCache);
    updatedDirCache = true;
  }
  return nullptr;
}

void FPackage::UnloadPackage(FPackage* package)
{
  if (!package)
  {
    return;
  }
  for (size_t idx = 0; idx < LoadedPackages.size(); ++idx)
  {
    if (LoadedPackages[idx].get() == package)
    {
      LoadedPackages.erase(LoadedPackages.begin() + idx);
      break;
    }
  }
}

FPackage::~FPackage()
{
  if (Stream)
  {
    delete Stream;
  }
  for (FObjectExport* exp : Exports)
  {
    delete exp;
  }
  for (FObjectImport* imp : Imports)
  {
    delete imp;
  }
  for (auto& obj : Objects)
  {
    delete obj.second;
  }
  for (auto& pkg : ExternalPackages)
  {
    UnloadPackage(pkg.get());
  }
  ExternalPackages.clear();
  if (Summary.DataPath != Summary.SourcePath)
  {
    std::filesystem::remove(std::filesystem::path(A2W(Summary.DataPath)));
  }
}

void FPackage::Initialize()
{
  Stream = new FReadStream(Summary.DataPath);
  Stream->SetPackage(this);
  FStream& s = GetStream();

  s.SetPosition(Summary.NamesOffset);

  for (uint32 idx = 0; idx < Summary.NamesCount; ++idx)
  {
    FNameEntry& e = Names.emplace_back(FNameEntry());
    s << e;
  }

  s.SetPosition(Summary.ImportsOffset);

  for (uint32 idx = 0; idx < Summary.ImportsCount; ++idx)
  {
    FObjectImport* imp = Imports.emplace_back(new FObjectImport(this));
    imp->ObjectIndex = -(PACKAGE_INDEX)idx - 1;
    s << *imp;
  }

  s.SetPosition(Summary.ExportsOffset);

  for (uint32 idx = 0; idx < Summary.ExportsCount; ++idx)
  {
    FObjectExport* exp = Exports.emplace_back(new FObjectExport(this));
    exp->ObjectIndex = (PACKAGE_INDEX)idx + 1;
    s << *exp;
  }

  s.SetPosition(Summary.DependsOffset);

  for (uint32 idx = 0; idx < Summary.ExportsCount; ++idx)
  {
    std::vector<int>& arr = Depends.emplace_back(std::vector<int>());
    s << arr;
  }

  for (uint32 idx = 0; idx < Summary.ImportsCount; ++idx)
  {
    FObjectImport* imp = Imports[idx];
    if (imp->OuterIndex)
    {
      FObjectImport* outer = GetImportObject(imp->OuterIndex);
      outer->Inner.push_back(imp);
    }
    else
    {
      RootImports.push_back(imp);
    }
  }

  for (uint32 index = 0; index < Summary.ExportsCount; ++index)
  {
    FObjectExport* exp = Exports[index];
#ifdef _DEBUG
    exp->ClassNameValue = exp->GetClassName();
#endif
    if (exp->OuterIndex)
    {
      FObjectExport* outer = GetExportObject(exp->OuterIndex);
      outer->Inner.push_back(exp);
    }
    else
    {
      RootExports.push_back(exp);
    }
    const std::string objName = exp->GetObjectName();
    if (!ObjectNameToExportMap.count(objName))
    {
      ObjectNameToExportMap[objName] = std::vector<FObjectExport*>();
    }
    ObjectNameToExportMap[objName].push_back(exp);
  }
  if (!(Summary.PackageFlags & PKG_ContainsScript))
  {
    for (uint32 idx = 0; idx < Summary.ExportsCount; ++idx)
    {
      FObjectExport* exp = Exports[idx];
      //std::cout << "Loading: " << exp->GetObjectName() << "(" << exp->GetClassName() << ")" << std::endl;
      UObject* obj = GetObject(exp);
      obj->Load();
    }
  }
  
}

std::string FPackage::GetPackageName(bool extension) const
{
  if (extension)
  {
    return Summary.PackageName;
  }
  else
  {
    std::string name = Summary.PackageName;
    if (name.length())
    {
      size_t start = 0;
      size_t idx = name.find_first_of(".", start);
      while (idx == 0)
      {
        start++;
        idx = name.find_first_of(".", start);
      }
      if (idx != std::string::npos)
      {
        name = name.substr(start, idx);
      }
    }
    return name;
  }
}

UObject* FPackage::GetObject(PACKAGE_INDEX index)
{
  if (Objects.count(index))
  {
    return Objects[index];
  }
  if (index > 0)
  {
    FObjectExport* exp = GetExportObject(index);
    if (exp->Object)
    {
      return exp->Object;
    }
    UObject* obj = UObject::Object(exp);
    Objects[index] = obj;

    return obj;
  }
  else if (index < 0)
  {
    FObjectImport* imp = GetImportObject(index);
    if (UObject* obj = GetObject(imp))
    {
      imp->Object = obj;
      return obj;
    }
  }
  return nullptr;
}

UObject* FPackage::GetObject(FObjectImport* imp)
{
  if (imp->Object || imp->DontLookUp)
  {
    return imp->Object;
  }
  if (imp->Package == this)
  {
    if (auto package = LoadPackageNamed(imp->GetPackageName()))
    {
      if (UObject* obj = package->GetObject(imp))
      {
        ExternalPackages.push_back(package);
        return obj;
      }
      UnloadPackage(package.get());
    }
    DLog("Couldn't find import %s(%s)", imp->GetObjectName().c_str(), imp->GetClassName().c_str());
    imp->DontLookUp = true;
    return nullptr;
  }
  std::vector<FObjectExport*> exps = GetExportObject(imp->GetObjectName());
  for (FObjectExport* exp : exps)
  {
    if (exp->GetClassName() == imp->GetClassName())
    {
      return GetObject(exp);
    }
  }
  imp->DontLookUp = true;
  return nullptr;
}

UObject* FPackage::GetObject(FObjectExport* exp)
{
  if (!Objects.count(exp->ObjectIndex))
  {
    Objects[exp->ObjectIndex] = UObject::Object(exp);
  }
  return Objects[exp->ObjectIndex];
}

PACKAGE_INDEX FPackage::GetObjectIndex(UObject* object)
{
  if (!object)
  {
    return 0;
  }
  if (object->GetPackage() != this)
  {
    return object->GetImportObject()->ObjectIndex;
  }
  return object->GetExportObject()->ObjectIndex;
}

UClass* FPackage::LoadClass(const std::string& className)
{
  for (FObjectImport* imp : Imports)
  {
    if (imp->GetObjectName() == className && imp->GetClassName() == "Class")
    {
      if (UObject* obj = imp->GetObject())
      {
        return (UClass*)obj;
      }
      if (imp->GetPackageName() == GetPackageName())
      {
        for (FObjectExport* exp : Exports)
        {
          if (exp->GetObjectName() == className && exp->GetClassName() == "Class")
          {
            imp->Object = GetObject(exp->ObjectIndex);
            return (UClass*)imp->Object;
          }
        }
        return nullptr;
      }
      UClass *cls = (UClass*)GetObject(imp);
      if (cls)
      {
        imp->Object = cls;
      }
      return cls;
    }
  }
  for (FObjectExport* exp : Exports)
  {
    if (exp->GetObjectName() == className && exp->GetClassName() == "Class")
    {
      return (UClass*)GetObject(exp->ObjectIndex);
    }
  }
  return nullptr;
}

void FPackage::AddNetObject(UObject* object)
{
  NetIndexMap[object->GetNetIndex()] = object;
}

std::vector<FObjectExport*> FPackage::GetExportObject(const std::string& name)
{
  if (ObjectNameToExportMap.count(name))
  {
    return ObjectNameToExportMap[name];
  }
  return std::vector<FObjectExport*>();
}