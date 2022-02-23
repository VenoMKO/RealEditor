#pragma once
#include <filesystem>
#include "FString.h"
#include "FStream.h"

extern const char* PackageMapperName;
extern const char* CompositePackageMapperName;
extern const char* ObjectRedirectorMapperName;

bool GEncrytMapperFile(const std::filesystem::path& path, const std::string& decrypted);
void GDecrytMapperFile(const std::filesystem::path& path, std::string& decrypted);

struct FCompositePackageMapEntry {
  FString ObjectPath;
  FString FileName;
  FILE_OFFSET Offset = 0;
  FILE_OFFSET Size = 0;

  friend FStream& operator<<(FStream& s, FCompositePackageMapEntry& e);
};

void DeserializeCompositePackageMapper(const std::filesystem::path& path, std::unordered_map<FString, FCompositePackageMapEntry>& outMap, std::unordered_map<FString, std::vector<FString>>& outList);
void SerializeCompositePackageMapper(const std::filesystem::path& path, const std::unordered_map<FString, FCompositePackageMapEntry>& map);

void DeserializePkgMapper(const std::filesystem::path& path, std::unordered_map<FString, FString>& outMap);

void DeserializeObjectRedirectorMapper(const std::filesystem::path& path, std::unordered_map<FString, FString>& outMap);