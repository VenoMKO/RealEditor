#pragma once
#include <string>
#include <vector>
#include <fstream>

// I want this to be independent from RE.

struct CompositeEntry {
  // Composite package name
  std::string CompositeName;
  // File storage name
  std::string Filename;
  // Primary objects path
  std::string Object;
  // Offset in the storage
  int Offset = 0;
  // Size in the storage
  int Size = 0;

  inline std::string ToString() const
  {
    return Object + ',' + CompositeName + ',' + std::to_string(Offset) + ',' + std::to_string(Size) + ",|";
  }
};

void GDecrytMapperFile(const std::wstring& path, std::string& output);
void GEncrytMapperFile(const std::wstring& path, const std::string& decrypted);

class CompositePatcher {
public:
  // Path - path to the .dat file.
  CompositePatcher(const std::wstring& path);
  
  // Read and decrypt .dat file. Throws.
  void Load();

  inline bool IsLoaded() const
  {
    return Loaded;
  }

  // Encrypt and write .dat file. Throws.
  void Apply();

  // Delete the entry (and file if the entry was the last one)
  bool DeleteEntry(const std::string& compositePackageName);

  // Patch and entry with the compositePackageName by using dest as a reference. Returns old entry with the filename. Throws.
  // If dest.Offset is 0, acts like a Delete. Removes an entry with the name compositePackageName without adding/changing anything
  std::string Patch(const std::string& compositePackageName, const CompositeEntry& dest);

private:
  bool Loaded = false;
  std::wstring Path;
  std::string Decrypted;
};