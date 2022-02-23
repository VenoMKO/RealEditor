#include "FMapper.h"
#include <array>

const char* PackageMapperName = "PkgMapper";
const char* CompositePackageMapperName = "CompositePackageMapper";
const char* ObjectRedirectorMapperName = "ObjectRedirectorMapper";

const char Key1[] = { 12, 6, 9, 4, 3, 14, 1, 10, 13, 2, 7, 15, 0, 8, 5, 11 };
const char Key2[] = { 'G', 'e', 'n', 'e', 'r', 'a', 't', 'e', 'P', 'a', 'c', 'k', 'a', 'g', 'e', 'M', 'a', 'p', 'p', 'e', 'r' };

// Reserves memory for CompositePackageMapper map
#define CPM_MAP_SIZE_GUESS 90
// Reserves memory for CompositePackageMapper list
#define CPM_LIST_SIZE_GUESS 450

FStream& operator<<(FStream& s, FCompositePackageMapEntry& e)
{
  s << e.ObjectPath;
  s << e.FileName;
  s << e.Offset;
  s << e.Size;
  return s;
}

bool GEncrytMapperFile(const std::filesystem::path& path, const std::string& decrypted)
{
  size_t size = decrypted.size();
  std::vector<char> encrypted(size);
  size_t offset = 0;
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
  const size_t end = size - sizeof(Key1);
  for (size_t offset = 0; offset <= end; offset += sizeof(Key1))
  {
    memcpy(&tmp[0], &encrypted[offset], sizeof(Key1));
    for (size_t idx = 0; idx < sizeof(Key1); ++idx)
    {
      encrypted[offset + idx] = tmp[Key1[idx]];
    }
  }

  std::ofstream s(path, std::ios::binary | std::ios::trunc);
  s.write(&encrypted[0], encrypted.size());
  return s.good();
}

void GDecrytMapperFile(const std::filesystem::path& path, std::string& decrypted)
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
    decrypted.resize(size);
    s.read(&encrypted[0], size);
  }

  size_t offset = 0;
  {
    const size_t end = size - sizeof(Key1);
    for (; offset <= end; offset += sizeof(Key1))
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

  if ((*(wchar*)decrypted.c_str()) == 0xFEFF)
  {
    decrypted = decrypted.substr(2);
  }
}

void DeserializeCompositePackageMapper(const std::filesystem::path& path, std::unordered_map<FString, FCompositePackageMapEntry>& outMap, std::unordered_map<FString, std::vector<FString>>& outList)
{
  FString buffer;
  GDecrytMapperFile(path, buffer.GetStorage());
  outMap.reserve(buffer.Size() / CPM_MAP_SIZE_GUESS);
  outList.reserve(CPM_LIST_SIZE_GUESS);

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
      outList[fileName].emplace_back(buffer.Substr(pos, elementEnd - pos));
      std::swap(pos, elementEnd);
      pos++;

      elementEnd = buffer.Find(',', pos);
      entry.Offset = (FILE_OFFSET)std::stoul(buffer.Substr(pos, elementEnd - pos).GetStorage());
      std::swap(pos, elementEnd);
      pos++;

      elementEnd = buffer.Find(',', pos);
      entry.Size = (FILE_OFFSET)std::stoul(buffer.Substr(pos, elementEnd - pos).GetStorage());
      std::swap(pos, elementEnd);
      pos++;

      DBreakIf(outMap.count(outList[fileName].back()));
      outMap[outList[fileName].back()] = entry;
    } while (buffer.Find('|', pos) < posEnd - 1);
    pos++;
    posEnd++;
  }
}

void SerializeCompositePackageMapper(const std::filesystem::path& path, const std::unordered_map<FString, FCompositePackageMapEntry>& map)
{
}

void DeserializePkgMapper(const std::filesystem::path& path, std::unordered_map<FString, FString>& outMap)
{
  FString buffer;
  GDecrytMapperFile(path, buffer.GetStorage());

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
    outMap.emplace(key, value);
    pos++;
    std::swap(pos, prevPos);
  }
}

void DeserializeObjectRedirectorMapper(const std::filesystem::path& path, std::unordered_map<FString, FString>& outMap)
{
  FString buffer;
  GDecrytMapperFile(path, buffer.GetStorage());

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
    outMap.emplace(key, value);
    pos++;
    std::swap(pos, prevPos);
  }
}
