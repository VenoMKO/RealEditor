#include "FPackage.h"
#include "ALog.h"
#include "FStream.h"
#include "FObjectResource.h"
#include "UObject.h"
#include "UClass.h" 

#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <ppl.h>

std::string FPackage::RootDir;
std::recursive_mutex FPackage::PackagesMutex;
std::vector<std::shared_ptr<FPackage>> FPackage::LoadedPackages;
std::vector<std::shared_ptr<FPackage>> FPackage::DefaultClassPackages;
std::vector<std::string> FPackage::DirCache;

namespace
{
  const char* CompositePackageMapperName = "CompositePackageMapper.dat.txt";
  const char* PackageListName = "Packages.list";
  const std::vector<std::string> DefaultClassPackageNames = { "Core.u", "Engine.u", "S1Game.u", "GameFramework.u", "GFxUI.u" };

  void BuildPackageList(const std::string& path, std::vector<std::string>& dirCache)
  {
    LogI("Building package list: \"%s\"", path.c_str());
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
    LogI("Done. Found %ld packages", dirCache.size());
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
    if (auto package = GetPackageNamed(name))
    {
      DefaultClassPackages.push_back(package);
    }
  }
  for (auto package : DefaultClassPackages)
  {
    package->Load();
  }
}

void FPackage::UnloadDefaultClassPackages()
{
  for (auto package : DefaultClassPackages)
  {
    UnloadPackage(package);
  }
  DefaultClassPackages.clear();
}

void FPackage::LoadCompositePackageMap()
{
  std::vector<std::string> packages;
  std::vector<std::string> mapping;
  {
    std::filesystem::path listPath(A2W(RootDir));
    listPath /= CompositePackageMapperName;

    std::string compositMapString;
    if (std::filesystem::exists(listPath))
    {
      std::ifstream s(listPath);
      if (s.is_open())
      {
        std::stringstream buffer;
        buffer << s.rdbuf();
        compositMapString = buffer.str();
      }
    }
    if (compositMapString.empty())
    {
      UThrow("Failed to load composit map! File CompositePackageMapper.dat.txt does not exist or empty!");
    }
    return;
    size_t pos = 0;
    size_t prevPos = 0;

    while ((pos = compositMapString.find('?', pos)) != std::string::npos)
    {
      packages.emplace_back(compositMapString.substr(prevPos, pos));
      prevPos = compositMapString.find('!', pos);
      mapping.emplace_back(compositMapString.substr(pos + 1, prevPos - pos - 1));
      std::swap(prevPos, pos);
    }
    Check(packages.size() == mapping.size());
  }
  
  // TODO: implement mapping
  // Probable struct: "UE3ObjectPath.ObjectName,PackageName,Offset,Size,|"
}

std::shared_ptr<FPackage> FPackage::GetPackage(const std::string& path, bool noInit)
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
  
  LogI("Opening package: %ls", A2W(path).c_str());
  FStream *stream = new FReadStream(path);
  if (!stream->IsGood())
  {
    UThrow("Couldn't open the file!");
  }
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
    LogI("Decompressing package %s to %ls", sum.PackageName.c_str(), decompressedPath.wstring().c_str());
    FStream* tempStream = new FWriteStream(sum.DataPath);

    std::vector<FCompressedChunk> tmpChunks;
    std::swap(sum.CompressedChunks, tmpChunks);
    (*tempStream) << sum;
    std::swap(sum.CompressedChunks, tmpChunks);

    uint8* decompressedData = (uint8*)malloc(totalDecompressedSize);
    try
    {
      concurrency::parallel_for(size_t(0), size_t(sum.CompressedChunks.size()), [sum, stream, compressedChunksData, decompressedData, startOffset](size_t idx) {
        const FCompressedChunk& chunk = sum.CompressedChunks[idx];
        uint8* dst = decompressedData + chunk.DecompressedOffset - startOffset;
        LZO::Decompress(compressedChunksData[idx], chunk.CompressedSize, dst, chunk.DecompressedSize);
      });
    }
    catch (const std::exception& e)
    {
      for (size_t idx = 0; idx < sum.CompressedChunks.size(); ++idx)
      {
        free(compressedChunksData[idx]);
      }
      delete[] compressedChunksData;
      free(decompressedData);
      delete tempStream;
      delete stream;
      throw e;
    }
    for (size_t idx = 0; idx < sum.CompressedChunks.size(); ++idx)
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
  }
  
  delete stream;
  std::shared_ptr<FPackage> result = nullptr;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
    result = LoadedPackages.emplace_back(new FPackage(sum));
  }
  return result;
}

std::shared_ptr<FPackage> FPackage::GetPackageNamed(const std::string& name, FGuid guid)
{
  std::shared_ptr<FPackage> found = nullptr;
  {
    std::scoped_lock<std::recursive_mutex> lock(PackagesMutex);
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
  }
  
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
      if (auto package = GetPackage(path))
      {
        if (!guid.IsValid() || guid == package->GetGuid())
        {
          return package;
        }
        UnloadPackage(package);
      }
    }
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
    LogI("Package %s unloaded", package->GetPackageName().c_str());
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
    UnloadPackage(pkg);
  }
  ExternalPackages.clear();
  if (Summary.DataPath != Summary.SourcePath)
  {
    std::filesystem::remove(std::filesystem::path(A2W(Summary.DataPath)));
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
  Names.clear();
  for (uint32 idx = 0; idx < Summary.NamesCount; ++idx)
  {
    FNameEntry& e = Names.emplace_back(FNameEntry());
    s << e;
    CheckCancel();
  }

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
#ifdef _DEBUG
    exp->ClassNameValue = exp->GetClassName();
#endif
    CheckCancel();
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

  for (uint32 index = 0; index < Summary.ExportsCount; ++index)
  {
    FObjectExport* exp = Exports[index];
    if (exp->OuterIndex)
    {
      FObjectExport* outer = GetExportObject(exp->OuterIndex);
      outer->Inner.push_back(exp);
    }
    else
    {
      RootExports.push_back(exp);
    }
    CheckCancel();
    const std::string objName = exp->GetObjectName();
    if (ObjectNameToExportMap[objName].empty())
    {
      ObjectNameToExportMap[objName].push_back(exp);
    }
  }

  for (uint32 idx = 0; idx < Summary.ImportsCount; ++idx)
  {
    CheckCancel();
    FObjectImport* imp = Imports[idx];
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

  Ready.store(true);
  Loading.store(false);
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
    if (imp->GetPackageName() != GetPackageName())
    {
      if (auto package = GetPackageNamed(imp->GetPackageName()))
      {
        package->Load();
        if (UObject* obj = package->GetObject(imp))
        {
          ExternalPackages.push_back(package);
          return obj;
        }
        UnloadPackage(package);
      }
      LogW("%s: Couldn't find import %s(%s)", GetPackageName().c_str(), imp->GetObjectName().c_str(), imp->GetClassName().c_str());
      imp->DontLookUp = true;
      return nullptr;
    }
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