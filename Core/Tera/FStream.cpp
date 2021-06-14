#include "FStream.h"
#include "FPackage.h"
#include "UObject.h"
#include "UClass.h"
#include "FObjectResource.h"

#include "Cast.h"
#include <Utils/ALog.h>

#include <ppl.h>

#define TRANS_UNRESOLVED 1000000
#define TRANS_NONE 1100000

FStream& FStream::operator<<(FString& s)
{
  if (IsReading())
  {
    s.Resize(0);
    int32 len = 0;
    (*this) << len;
    if (len > 0)
    {
      s.Resize(len);
      SerializeBytes(&s[0], len);
    }
    else if (len < 0)
    {
      std::wstring wstr;
      wstr.resize(-len);
      len = -len * 2;
      SerializeBytes(&wstr[0], len);
      s += W2A(wstr);
    }
  }
  else
  {
    int32 len = (int32)s.Size();
    if (!s.IsAnsi())
    {
      std::wstring wstr = s;
      len = -(int32)wstr.size();
      (*this) << len;
      SerializeBytes((void*)wstr.c_str(), len * -2);
    }
    else
    {
      (*this) << len;
      SerializeBytes((void*)s.C_str(), len);
    }
  }
  return *this;
}

FStream& FStream::operator<<(FName& n)
{
  if (IsReading() && GetPackage())
  {
    n.SetPackage(GetPackage());
  }
  
  SerializeNameIndex(n);
  int32 number = n.Number;
  (*this) << number;
  if (IsReading())
  {
    n.Number = number;
  }

#ifdef _DEBUG
  n.Value = n.String();
#endif
  return *this;
}

FStream& FStream::SerializeObjectArray(std::vector<class UObject*>& objects, std::vector<PACKAGE_INDEX>& indices)
{
  int32 cnt = static_cast<int32>(objects.size());
  if (IsReading())
  {
    (*this) << cnt;
    objects.reserve(cnt);
    indices.reserve(cnt);
    for (int32 idx = 0; idx < cnt; ++idx)
    {
      void* obj = nullptr;
      PACKAGE_INDEX& objIdx = indices.emplace_back(0);
      SerializeObjectRef(obj, objIdx);
      objects.emplace_back((UObject*)obj);
    }
  }
  else
  {
    (*this) << cnt;
    for (int32 idx = 0; idx < cnt; ++idx)
    {
      void* obj = (void*)objects[idx];
      PACKAGE_INDEX objIdx = indices[idx];
      SerializeObjectRef(obj, objIdx);
    }
  }
  return *this;
}

FStream& FStream::SerializeNameIndex(FName& n)
{
  NAME_INDEX idx = -1;
  if (IsReading())
  {
    (*this) << n.Index;
  }
  else
  {
    idx = Package->GetNameIndex(n.String(false), false);
    DBreakIf(idx < 0);
    (*this) << idx;
  }
  return *this;
}

uint16 FStream::GetFV() const
{
  return Package ? Package->GetFileVersion() : 0;
}

uint16 FStream::GetLV() const
{
  return Package ? Package->GetLicenseeVersion() : 0;
}

FStream& FStream::operator<<(FStringRef& r)
{
  if (IsReading())
  {
    r.Package = GetPackage();
    r.Offset = GetPosition();
    (*this) << r.Size;
    SetPosition(r.Offset + sizeof(FILE_OFFSET) + r.Size);
  }
  else
  {
    if (r.Cached->Empty())
    {
      r.GetString();
    }
    (*this) << *r.Cached;
  }
  return *this;
}

void FStream::SerializeObjectRef(void*& obj, PACKAGE_INDEX& index)
{
  if (IsReading())
  {
    (*this) << index;
    FILE_OFFSET tmpPos = GetPosition();
    obj = GetPackage()->GetObject(index, GetLoadSerializedObjects());
    SetPosition(tmpPos);
  }
  else
  {
    if (obj)
    {
#if _DEBUG
      PACKAGE_INDEX idx = GetPackage()->GetObjectIndex((UObject*&)obj);
      DBreakIf(idx != index);
      index = idx;
#else
      index = GetPackage()->GetObjectIndex((UObject*&)obj);
#endif
    }
    (*this) << index;
  }
}

void FStream::SerializeCompressed(void* v, int32 length, ECompressionFlags flags, bool concurrent)
{
  if (IsReading())
  {
    FCompressedChunkInfo packageFileTag;
    *this << packageFileTag;

    if (packageFileTag.CompressedSize != PACKAGE_MAGIC)
    {
      UThrow("Failed to decompress data! No compressed magic!");
    }

    FCompressedChunkInfo summary;
    *this << summary;

    int32 loadingCompressionChunkSize = packageFileTag.DecompressedSize;
    if (loadingCompressionChunkSize == PACKAGE_MAGIC)
    {
      loadingCompressionChunkSize = COMPRESSED_BLOCK_SIZE;
    }

    int32	totalChunkCount = (summary.DecompressedSize + loadingCompressionChunkSize - 1) / loadingCompressionChunkSize;
    FCompressedChunkInfo* chunkInfo = new FCompressedChunkInfo[totalChunkCount];

    if (concurrent && totalChunkCount > 1)
    {
      void** compressedDataChunks = new void*[totalChunkCount];
      for (int32 idx = 0; idx < totalChunkCount; ++idx)
      {
        *this << chunkInfo[idx];
        if (idx)
        {
          chunkInfo[idx].DecompressedOffset = chunkInfo[idx - 1].DecompressedOffset + chunkInfo[idx - 1].DecompressedSize;
        }
      }
      for (int32 idx = 0; idx < totalChunkCount; ++idx)
      {
        compressedDataChunks[idx] = malloc(chunkInfo[idx].CompressedSize);
        SerializeBytes(compressedDataChunks[idx], chunkInfo[idx].CompressedSize);
      }

      uint8* dest = (uint8*)v;
      std::atomic_bool err = {false};
      concurrency::parallel_for(int32(0), totalChunkCount, [&](int32 idx) {
        const FCompressedChunkInfo& chunk = chunkInfo[idx];
        if (err.load())
        {
          return;
        }
        if (!DecompressMemory(flags, dest + chunk.DecompressedOffset, chunk.DecompressedSize, compressedDataChunks[idx], chunk.CompressedSize))
        {
          err.store(true);
        }
      });

      for (int32 idx = 0; idx < totalChunkCount; ++idx)
      {
        free(compressedDataChunks[idx]);
      }

      delete[] chunkInfo;
      delete[] compressedDataChunks;

      if (err.load())
      {
        UThrow("Failed to decompress data!");
      }
    }
    else
    {
      int32 maxCompressedSize = 0;
      for (int32 idx = 0; idx < totalChunkCount; ++idx)
      {
        *this << chunkInfo[idx];
        maxCompressedSize = std::max(chunkInfo[idx].CompressedSize, maxCompressedSize);
      }

      uint8* dest = (uint8*)v;
      void* compressedBuffer = malloc(maxCompressedSize);
      bool err = false;
      for (int32 idx = 0; idx < totalChunkCount; ++idx)
      {
        const FCompressedChunkInfo& chunk = chunkInfo[idx];
        SerializeBytes(compressedBuffer, chunk.CompressedSize);
        if (!DecompressMemory(flags, dest, chunk.DecompressedSize, compressedBuffer, chunk.CompressedSize))
        {
          err = true;
          break;
        }
        dest += chunk.DecompressedSize;
      }

      free(compressedBuffer);
      delete[] chunkInfo;

      if (err)
      {
        UThrow("Failed to decompress data!");
      }
    }
  }
  else
  {
    FCompressedChunkInfo packageFileTag;
    packageFileTag.CompressedSize = PACKAGE_MAGIC;
    packageFileTag.DecompressedSize = COMPRESSED_BLOCK_SIZE;
    *this << packageFileTag;

int32	chunkCount = (length + COMPRESSED_BLOCK_SIZE - 1) / COMPRESSED_BLOCK_SIZE + 1;
FILE_OFFSET startPosition = GetPosition();
FCompressedChunkInfo* compressionChunks = new FCompressedChunkInfo[chunkCount];

for (int32 idx = 0; idx < chunkCount; idx++)
{
  *this << compressionChunks[idx];
}

compressionChunks[0].DecompressedSize = length;
compressionChunks[0].CompressedSize = 0;

int32 remainingSize = length;
int32 chunkIndex = 1;
int32 bufferSize = 2 * COMPRESSED_BLOCK_SIZE;
void* buffer = malloc(bufferSize);

uint8* src = (uint8*)v;
while (remainingSize > 0)
{
  int32 sizeToCompress = std::min(remainingSize, COMPRESSED_BLOCK_SIZE);
  int32 compressedSize = bufferSize;

  CompressMemory(flags, buffer, &compressedSize, src, sizeToCompress);
  src += sizeToCompress;

  SerializeBytes(buffer, compressedSize);
  compressionChunks[0].CompressedSize += compressedSize;
  compressionChunks[chunkIndex].CompressedSize = compressedSize;
  compressionChunks[chunkIndex].DecompressedSize = sizeToCompress;
  chunkIndex++;

  remainingSize -= COMPRESSED_BLOCK_SIZE;
}
free(buffer);

FILE_OFFSET endPosition = GetPosition();
SetPosition(startPosition);

for (int32 ChunkIndex = 0; ChunkIndex < chunkCount; ChunkIndex++)
{
  *this << compressionChunks[ChunkIndex];
}
delete[] compressionChunks;
SetPosition(endPosition);
  }
}

void MTransStream::SerializeBytes(void* ptr, FILE_OFFSET size)
{
  if (IsReading())
  {
    if (Position + size > DataSize)
    {
      Good = false;
      DBreak();
      return;
    }
    std::memcpy(ptr, Data + Position, size);
    Position += size;
  }
  else
  {
    while (Position + size >= AllocationSize)
    {
      if (void* tmp = realloc(Data, AllocationSize + BufferSize))
      {
        Data = (uint8*)tmp;
        AllocationSize += BufferSize;
      }
    }
    std::memcpy(Data + Position, ptr, size);
    Position += size;
    if (Position > DataSize)
    {
      DataSize = Position;
    }
  }
}

void MTransStream::SerializeObjectRef(void*& obj, PACKAGE_INDEX& index)
{
  PACKAGE_INDEX idx = 0;
  if (IsReading())
  {
    (*this) << idx;

    if (idx == TRANS_NONE)
    {
      obj = nullptr;
      index = 0;
    }
    else if (idx >= TRANS_UNRESOLVED)
    {
      DBreak();
    }
    else if (idx < 0)
    {
      if (ImportsMap[Imports[idx]])
      {
        obj = DestPackage->GetObject(ImportsMap[Imports[idx]]);
        index = ImportsMap[Imports[idx]]->ObjectIndex;
      }
      else
      {
        DBreak();
      }
    }
    else
    {
      obj = DependsMap[Depends[idx]];
      index = DestPackage->GetObjectIndex((UObject*)obj);
    }
  }
  else
  {
    PACKAGE_INDEX idx = index;
    if (obj)
    {
      auto it = std::find(Depends.begin(), Depends.end(), obj);
      if (it == Depends.end())
      {
        idx = (PACKAGE_INDEX)Depends.size();
        Depends.push_back((UObject*)obj);
      }
      else
      {
        idx = (PACKAGE_INDEX)(it - Depends.begin());
      }
    }
    else
    {
      if (idx)
      {
        DBreak();
        FObjectResource *res = SourcePackage->GetResourceObject(idx);
        if (idx < 0)
        {
          FObjectImport* imp = SourcePackage->GetImportObject(idx);
          idx = -(PACKAGE_INDEX)Imports.size() - 1;
          Imports.emplace_back(imp);
        }
        else
        {
          FObjectExport* exp = SourcePackage->GetExportObject(idx);
          idx = TRANS_UNRESOLVED + (PACKAGE_INDEX)Unresolved.size();
          Unresolved.emplace_back(exp);
        }
      }
      else
      {
        idx = TRANS_NONE;
      }
    }
    (*this) << idx;
  }
}

FStream& MTransStream::SerializeNameIndex(class FName& n)
{
  NAME_INDEX idx = -1;
  if (IsReading())
  {
    (*this) << idx;
    n.SetIndex(DestPackage->GetNameIndex(Names[idx], true));
  }
  else
  {
    const FString str = n.String(false);
    auto it = std::find(Names.begin(), Names.end(), str);
    if (it == Names.end())
    {
      idx = (NAME_INDEX)Names.size();
      Names.emplace_back(str);
    }
    else
    {
      idx = (NAME_INDEX)(it - Names.begin());
    }
    DBreakIf(idx == INDEX_NONE);
    (*this) << idx;
  }
  return *this;
}

void MTransStream::SetSourcePackage(std::shared_ptr<FPackage> package)
{
  SourcePackage = package;
}

void MTransStream::SetDestinationPackage(FPackage* package)
{
  DestPackage = package;
  OrphansMap.clear();
  if (DestPackage)
  {
    for (UObject* obj : Depends)
    {
      if (obj == Root)
      {
        continue;
      }
      if (SourcePackage->GetObjectIndex(obj) < 0)
      {
        // Skip imports
        continue;
      }
      UObject* outer = obj->GetOuter();
      bool hasParent = false;
      while (outer)
      {
        if (outer == Root)
        {
          hasParent = true;
          break;
        }
        if (std::find(Depends.begin(), Depends.end(), outer) != Depends.end())
        {
          hasParent = true;
          break;
        }
        outer = outer->GetOuter();
      }
      if (hasParent)
      {
        continue;
      }
      OrphansMap[obj] = FindDestinationObject(obj);
    }
  }
}

uint16 MTransStream::GetFV() const
{
  if (IsReading())
  {
    return DestPackage ? DestPackage->GetFileVersion() : 0;
  }
  return SourcePackage ? SourcePackage->GetFileVersion() : 0;
}

uint16 MTransStream::GetLV() const
{
  if (IsReading())
  {
    return DestPackage ? DestPackage->GetLicenseeVersion() : 0;
  }
  return SourcePackage ? SourcePackage->GetLicenseeVersion() : 0;
}

bool MTransStream::StartObjectTransaction(class UObject* obj)
{
  Clear();
  ErrorText.Clear();
  if (!obj)
  {
    return false;
  }
  Reading = false;
  Root = obj;
  SourcePackage = obj->GetPackage()->Ref();
  bool result = SerializeObject(obj, true);
  while (result)
  {
    ClearData();
    size_t cnt = Depends.size();
    for (int32 idx = cnt - 1; idx >= 0 && result; --idx)
    {
      result = SerializeObject(Depends[idx], true);
    }
    if (cnt == Depends.size())
    {
      // Keep serializing until all objects found
      break;
    }
  }
  if (DestPackage)
  {
    // Find orphans
    SetDestinationPackage(DestPackage);
  }
  if (Unresolved.size())
  {
    Clear();
    ErrorText = "Failed to resolve some of the objects.";
    return false;
  }
  if (!result)
  {
    Clear();
  }
  return result;
}

bool MTransStream::EndObjectTransaction(class UObject* obj)
{
  for (const FString& name : Names)
  {
    DestPackage->GetNameIndex(name, true);
  }

  UObject* newRoot = nullptr;
  if (!obj || obj->GetClassName() == NAME_Package)
  {
    FString name = CustomRootName.Empty() ? Root->GetObjectNameString() : FString(CustomRootName.WString());
    FObjectExport* exp = DestPackage->AddExport(name, Root->GetClassNameString(), obj ? obj->GetExportObject() : nullptr);
    newRoot = DestPackage->GetObject(exp, false);
  }
  else
  {
    // TOOD: implement
    // Replace the obj with the new object
    DBreak();
  }

  std::map<UObject*, UObject*> map;
  for (UObject* dep : Depends)
  {
    if (dep == Root)
    {
      map[dep] = newRoot;
      continue;
    }
    if (map[dep])
    {
      // Already mapped
      continue;
    }
    if (DestPackage->GetObjectIndex(dep) > 0)
    {
      // The object exists in the dest package
      bool isChildOfRoot = false;
      UObject* outer = dep->GetOuter();
      while (outer)
      {
        if (outer == Root)
        {
          isChildOfRoot = true;
          break;
        }
        outer = outer->GetOuter();
      }
      // Skip inner of the root in the dest package
      if (!isChildOfRoot)
      {
        map[dep] = dep;
        continue;
      }
    }
    if (SourcePackage->GetObjectIndex(dep) < 0)
    {
      // Import
      if (dep->GetClassName() == NAME_Class)
      {
        DestPackage->ImportClass(Cast<UClass>(dep));
        map[dep] = dep;
        continue;
      }
      // TODO: composite packages don't work well with imports. Need to pull imports to the DestPackages' exports
      ErrorText = FString::Sprintf("Can't copy the import object: %s", dep->GetObjectNameString().UTF8().c_str());
      return false;
      /*
      FObjectImport* imp = nullptr;
      DestPackage->AddImport(dep, imp);
      DBreakIf(!imp);
      map[dep] = DestPackage->GetObject(imp);
      continue;*/
    }
    if (OrphansMap.count(dep))
    {
      // Map via the orphan map
      UObject* mapped = OrphansMap[dep];
      if (mapped->GetClassName() != NAME_Package)
      {
        map[dep] = mapped;
        continue;
      }
      FObjectExport* exp = DestPackage->AddExport(dep->GetObjectNameString(), dep->GetClassNameString(), mapped->GetExportObject());
      DBreakIf(!exp);
      map[dep] = DestPackage->GetObject(exp, false);
      continue;
    }
    if (!dep->GetOuter())
    {
      // Attach to the root export
      FObjectExport* exp = DestPackage->AddExport(dep->GetObjectNameString(), dep->GetClassNameString(), nullptr);
      DBreakIf(!exp);
      map[dep] = DestPackage->GetObject(exp, false);
      continue;
    }
    if (dep->GetOuter() == Root)
    {
      // Root object hosts the dep object
      FObjectExport* exp = DestPackage->AddExport(dep->GetObjectNameString(), dep->GetClassNameString(), map[Root]->GetExportObject());
      DBreakIf(!exp);
      map[dep] = DestPackage->GetObject(exp, false);
      continue;
    }
    // Find parent
    std::vector<UObject*> missing;
    UObject* tmp = dep;
    while (tmp->GetOuter() && !map[tmp->GetOuter()])
    {
      missing.insert(missing.begin(), tmp);
      tmp = tmp->GetOuter();
    }
    DBreakIf(!tmp); // We must hit the root object or its child
    for (UObject* parent : missing)
    {
      DBreakIf(!map[parent->GetOuter()]);
      FObjectExport* exp = DestPackage->AddExport(parent->GetObjectNameString(), parent->GetClassNameString(), map[parent->GetOuter()]->GetExportObject());
      DBreakIf(!exp);
      map[parent] = DestPackage->GetObject(exp, false);
    }
    FObjectExport* exp = DestPackage->AddExport(dep->GetObjectNameString(), dep->GetClassNameString(), map[dep->GetOuter()]->GetExportObject());
    DBreakIf(!exp);
    map[dep] = DestPackage->GetObject(exp, false);
  }
  DependsMap = map;

  // At this point all Depends must exist in the DestPackage
  for (UObject* dep : Depends)
  {
    if (!map[dep])
    {
      DBreak();
      ErrorText = FString::Sprintf("Error! Failed to map %s object.", dep->GetFullObjectName().UTF8().c_str());
      Clear();
      return false;
    }
  }

  for (UObject* dep : Depends)
  {
    UObject* mapped = map[dep];
    DBreakIf(!mapped);
    if (DestPackage->GetObjectIndex(mapped) < 0 || mapped->GetExportObject()->SerialOffset)
    {
      continue;
    }
    free(Data);
    Data = nullptr;
    Position = 0;
    DataSize = 0;
    AllocationSize = 0;
    
    Reading = false;

    DBreakIf(!dep->GetExportObject());
    dep->SetTransacting(true);
    dep->Serialize(*this);
    dep->SetTransacting(false);
    
    Position = 0;
    Reading = true;
    
    
    mapped->SetTransacting(true);
    mapped->Serialize(*this);
    mapped->SetTransacting(false);
  }

  for (UObject* dep : Depends)
  {
    if (DependsMap[dep]->GetExportObject()->SerialOffset)
    {
      continue;
    }
    if (DependsMap[dep]->GetPackage() == DestPackage)
    {
      DependsMap[dep]->GetExportObject()->ObjectFlags = dep->GetObjectFlags();
      DependsMap[dep]->MarkDirty();
    }
  }
  return true;
}

UObject* MTransStream::GetTransactedObject() const
{
  return DependsMap.at(Root);
}

void MTransStream::Clear()
{
  ClearData();
  Root = nullptr;
  CustomRootName.Clear();
  Good = true;
  SourcePackage = nullptr;
  DestPackage = nullptr;
  Names.clear();
  Depends.clear();
  Imports.clear();
  Unresolved.clear();
  OrphansMap.clear();
  DependsMap.clear();
  ImportsMap.clear();
  UnresolvedRemap.clear();
}

void MTransStream::ClearData()
{
  free(Data);
  Data = nullptr;
  Position = 0;
  AllocationSize = 0;
  DataSize = 0;
}

std::map<UObject*, UObject*> MTransStream::GetOrphanObjects()
{
  std::map<UObject*, UObject*> result;
  for (auto& p : OrphansMap)
  {
    if (p.second && p.second->GetObjectName() == p.first->GetObjectName())
    {
      continue;
    }
    result[p.first] = p.second;
  }
  return result;
}

void MTransStream::ApplyOrphansMap(const std::map<UObject*, UObject*>& map)
{
  for (const auto& p : map)
  {
    OrphansMap[p.first] = p.second;
  }
}

UObject* MTransStream::FindDestinationObject(UObject* obj) const
{
  if (!obj)
  {
    return nullptr;
  }
  std::vector<FObjectExport*> exports = DestPackage->GetAllExports();
  const FString testName = obj->GetObjectNameString().ToUpper();
  const FString testClass = obj->GetClassNameString();
  for (FObjectExport* exp : exports)
  {
    if ((exp->GetObjectNameString().ToUpper().StartWith(testName) || testName.StartWith(exp->GetObjectNameString().ToUpper())) && exp->GetClassName() == testClass)
    {
      if (UObject* result = DestPackage->GetObject(exp))
      {
        result->Load();
        return result;
      }
    }
  }
  std::vector<FObjectImport*> imports = DestPackage->GetAllImports();
  for (FObjectImport* imp : imports)
  {
    if ((imp->GetObjectNameString().ToUpper().StartWith(testName) || testName.StartWith(imp->GetObjectNameString().ToUpper())) && imp->GetClassNameString() == testClass)
    {
      if (UObject* result = DestPackage->GetObject(imp))
      {
        result->Load();
        return result;
      }
    }
  }
  return nullptr;
}

bool MTransStream::SerializeObject(UObject* obj, bool recursiv)
{
  if (!obj)
  {
    return false;
  }
  try
  {
    obj->Load();
  }
  catch (const std::exception& e)
  {
    ErrorText = FString::Sprintf("Failed to load the object \"%s\": %s", obj->GetObjectNameString().UTF8().c_str(), e.what());
    return false;
  }
  try
  {
    if (std::find(Depends.begin(), Depends.end(), obj) == Depends.end())
    {
      Depends.emplace_back(obj);
    }
    
    if (recursiv)
    {
      auto children = obj->GetInner();
      for (UObject* inner : children)
      {
        if (!SerializeObject(inner, recursiv))
        {
          return false;
        }
      }
    }
    
    if (std::find(Names.begin(), Names.end(), obj->GetObjectNameString()) == Names.end())
    {
      Names.emplace_back(obj->GetObjectNameString());
    }
    if (std::find(Names.begin(), Names.end(), obj->GetClassNameString()) == Names.end())
    {
      Names.emplace_back(obj->GetClassNameString());
    }
    obj->SetTransacting(true);
    obj->Serialize(*this);
    obj->SetTransacting(false);
  }
  catch (const std::exception& e)
  {
    obj->SetTransacting(false);
    Good = false;
    ErrorText = FString::Sprintf("Failed to copy the object \"%s\": %s", obj->GetObjectNameString().UTF8().c_str(), e.what());
    return false;
  }
  {
    FString name = obj->GetObjectNameString();
    if (std::find(Names.begin(), Names.end(), name) == Names.end())
    {
      Names.emplace_back(name);
    }
    name = obj->GetClassNameString();
    if (std::find(Names.begin(), Names.end(), name) == Names.end())
    {
      Names.emplace_back(name);
    }
  }
  return true;
}

bool MTransStream::DeserializeObject(class UObject* obj)
{
  return false;
}
