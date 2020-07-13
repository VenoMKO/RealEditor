#pragma once
#include "Core.h"
#include "FName.h"
#include "FStructs.h"
#include <memory>

class FPackage {
public:

	// Load class packages
	static void LoadDefaultClassPackages();
	// Unload class packages
	static void UnloadDefaultClassPackages();
	// Load and retain a package at the path. Every LoadPackage call must pair a UnloadPackage call
	static std::shared_ptr<FPackage> LoadPackage(const std::string& path);
	// Load and retain a package by name and guid(if valid). Every LoadPackageNamed call must pair a UnloadPackage call
	static std::shared_ptr<FPackage> LoadPackageNamed(const std::string& name, FGuid guid = FGuid());
	// Release a package. 
	static void UnloadPackage(FPackage* package);
	// Find and cache all packages
	static void SetRootPath(const std::string& path);

private:
	// Packages must be loaded/created from the static methods
	FPackage(FPackageSummary& sum)
		: Summary(sum)
	{}

public:
	~FPackage();

	// Create a read stream using DataPath and serialize tables
	void Initialize();

	// Get an object at index
	UObject* GetObject(PACKAGE_INDEX index);

	// Import an object from a different package.
	UObject* GetObject(FObjectImport* imp);
	
	// Get an object
	UObject* GetObject(FObjectExport* exp);

	// Get package index of the object. Accepts imported objects
	PACKAGE_INDEX GetObjectIndex(UObject* object);

	// Get a UClass with name className
	UClass* LoadClass(const std::string& className);

	// Cache UObject with a netIndex to NetIndexMap
	void AddNetObject(UObject* object);

	// Get all exports that match the name
	std::vector<FObjectExport*> GetExportObject(const std::string& name);

	// Get all root exports
	inline std::vector<FObjectExport*> GetRootExports()
	{
		return RootExports;
	}

	// Get all root imports
	inline std::vector<FObjectImport*> GetRootImports()
	{
		return RootImports;
	}

	// Get import/export object at index
	inline FObjectResource* GetResourceObject(PACKAGE_INDEX index)
	{
		if (index < 0)
		{
			return (FObjectResource*)GetImportObject(index);
		}
		return (FObjectResource*)GetExportObject(index);
	}

	// Get import object at index
	FObjectImport* GetImportObject(PACKAGE_INDEX index)
	{
		const PACKAGE_INDEX i = -index - 1;
		return Imports[i];
	}

	// Get export object at index
	FObjectExport* GetExportObject(PACKAGE_INDEX index)
	{
		const PACKAGE_INDEX i = index - 1;
		return Exports[i];
	}

	// Get package read stream
	FStream& GetStream()
	{
		return *Stream;
	}

	// Get package guid
	FGuid GetGuid() const
	{
		return Summary.Guid;
	}

	// Get name at index
	std::string GetIndexedName(NAME_INDEX index) const
	{
		return Names[index].GetString();
	}

	// Get package's source path(may differ from a DataPath)
	std::string GetSourcePath() const
	{
		return Summary.SourcePath;
	}

	// Get package name
	std::string GetPackageName(bool extension = false) const;

private:
	FPackageSummary Summary;
	FStream* Stream = nullptr;

	std::vector<FNameEntry> Names;
	std::vector<FObjectExport*> Exports;
	std::vector<FObjectImport*> Imports;
	std::vector<std::vector<int32>> Depends;
	std::map<PACKAGE_INDEX, UObject*> Objects;

	std::vector<FObjectExport*> RootExports;
	std::vector<FObjectImport*> RootImports;

	// Cached netIndices for faster netIndex lookup. Containes only loaded objects!
	std::map<NET_INDEX, UObject*> NetIndexMap;
	// Name to Object map for faster import lookup
	std::map<std::string, std::vector<FObjectExport*>> ObjectNameToExportMap;
	// List of packages we rely on
	std::vector<std::shared_ptr<FPackage>> ExternalPackages;

	static std::string RootDir;
	static std::vector<std::shared_ptr<FPackage>> LoadedPackages;
	static std::vector<std::shared_ptr<FPackage>> DefaultClassPackages;
	static std::vector<std::string> DirCache;
};