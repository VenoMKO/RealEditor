#pragma once
#include "Core.h"
#include "FName.h"
#include "FStructs.h"

#include <memory>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

class FPackage {
public:

	// RootDir's Core.u version
	static uint16 GetCoreVersion();
	// Load Cooked Persistent Data
	static void LoadPersistentData();
	// Load class packages
	static void LoadClassPackage(const FString& name);
	// Unload class packages
	static void UnloadDefaultClassPackages();
	// Load Package Map
	static void LoadPkgMapper();
	// Load Composite Package Map
	static void LoadCompositePackageMapper();
	// Load ObjectReference Map
	static void LoadObjectRedirectorMapper();
	// Load and retain a package at the path. Every GetPackage call must pair a UnloadPackage call
	static std::shared_ptr<FPackage> GetPackage(const FString& path);
	// Load and retain a package by name and guid(if valid). Every GetPackageNamed call must pair a UnloadPackage call
	static std::shared_ptr<FPackage> GetPackageNamed(const FString& name, FGuid guid = FGuid());
	// Release a package. 
	static void UnloadPackage(std::shared_ptr<FPackage> package);
	// Find and cache all packages
	static void SetRootPath(const FString& path);

private:
	// Packages must be loaded/created from the static methods
	FPackage(FPackageSummary& sum)
		: Summary(sum)
	{}

public:
	~FPackage();

	// Create a read stream using DataPath and serialize tables
	void Load();

	// Get an object at index
	UObject* GetObject(PACKAGE_INDEX index, bool load = true);

	// Import an object from a different package.
	UObject* GetObject(FObjectImport* imp, bool load = true);
	
	// Get an object
	UObject* GetObject(FObjectExport* exp, bool load = true);

	// Get package index of the object. Accepts imported objects
	PACKAGE_INDEX GetObjectIndex(UObject* object) const;

	PACKAGE_INDEX GetNameIndex(const FString& name, bool insert = false);

	// Get a UClass with idx;
	UClass* LoadClass(PACKAGE_INDEX index);

	// Cache UObject with a netIndex to NetIndexMap
	void AddNetObject(UObject* object);

	// Get an object by net index and name
	UObject* GetObject(NET_INDEX netIndex, const FString& name, const FString& className);

	// Get all exports that match the name
	std::vector<FObjectExport*> GetExportObject(const FString& name);

	// Get all root exports
	inline std::vector<FObjectExport*> GetRootExports() const
	{
		return RootExports;
	}

	// Get all root imports
	inline std::vector<FObjectImport*> GetRootImports() const
	{
		return RootImports;
	}

	inline std::vector<FObjectExport*> GetAllExports() const
	{
		return Exports;
	}

	inline std::vector<FObjectImport*> GetAllImports() const
	{
		return Imports;
	}

	// Get import/export object at index
	inline FObjectResource* GetResourceObject(PACKAGE_INDEX index) const
	{
		if (index < 0)
		{
			return (FObjectResource*)GetImportObject(index);
		}
		return (FObjectResource*)GetExportObject(index);
	}

	// Get import object at index
	inline FObjectImport* GetImportObject(PACKAGE_INDEX index) const
	{
		const PACKAGE_INDEX i = -index - 1;
		return Imports[i];
	}

	FObjectImport* GetImportObject(const FString& objectName, const FString& className) const;

	// Get export object at index
	inline FObjectExport* GetExportObject(PACKAGE_INDEX index) const
	{
		const PACKAGE_INDEX i = index - 1;
		return Exports[i];
	}

	// Get package read stream
	inline FStream& GetStream()
	{
		return *Stream;
	}

	// Get package guid
	inline FGuid GetGuid() const
	{
		return Summary.Guid;
	}

	// Get name at index
	inline FString GetIndexedName(NAME_INDEX index) const
	{
		return Names[index].GetString();
	}

	// Get name at index
	inline void GetIndexedName(NAME_INDEX index, FString& output) const
	{
		Names[index].GetString(output);
	}

	// Get package's source path(may differ from a DataPath)
	inline FString GetSourcePath() const
	{
		return Summary.SourcePath;
	}

	// Get package's source path(may differ from a DataPath)
	inline FString GetDataPath() const
	{
		return Summary.DataPath;
	}

	// Returns true if Load() finished
	inline bool IsReady() const
	{
		return Ready.load();
	}

	// Cancell loading operation
	void CancelOperation()
	{
		Cancelled.store(true);
	}

	bool IsOperationCancelled() const
	{
		return Cancelled.load();
	}

	inline uint16 GetFileVersion() const
	{
		return Summary.GetFileVersion();
	}

	inline uint16 GetLicenseeVersion() const
	{
		return Summary.GetLicenseeVersion();
	}
	
	// Used to create builtin classes
	VObjectExport* CreateVirtualExport(const char* objName, const char* clsName);

	// Get package name
	FString GetPackageName(bool extension = false) const;

private:
	void _DebugDump() const;

	UObject* GetCachedExportObject(PACKAGE_INDEX index) const;
	UObject* GetCachedForcedObject(PACKAGE_INDEX index) const;
	UObject* GetCachedImportObject(PACKAGE_INDEX index) const;

	UObject* SetCachedExportObject(PACKAGE_INDEX index, UObject* obj);
	UObject* SetCachedForcedObject(PACKAGE_INDEX index, UObject* obj);
	UObject* SetCachedImportObject(PACKAGE_INDEX index, UObject* obj);

	UObject* GetForcedExport(FObjectExport* exp);

private:
	FPackageSummary Summary;
	FStream* Stream = nullptr;

	// Prevent multiple Load() calls
	std::atomic_bool Loading = { false };
	// Load finished 
	std::atomic_bool Ready = { false };
	// Load was cancelled
	std::atomic_bool Cancelled = { false };

	std::vector<FNameEntry> Names;
	std::vector<FObjectExport*> Exports;
	std::vector<VObjectExport*> VExports;
	std::vector<FObjectImport*> Imports;
	std::vector<std::vector<int32>> Depends;
	
	mutable std::mutex ExportObjectsMutex;
	std::unordered_map<PACKAGE_INDEX, UObject*> ExportObjects;
	mutable std::mutex ForcedObjectsMutex;
	std::unordered_map<PACKAGE_INDEX, UObject*> ForcedObjects;
	mutable std::mutex ImportObjectsMutex;
	std::unordered_map<PACKAGE_INDEX, UObject*> ImportObjects;

	std::vector<FObjectExport*> RootExports;
	std::vector<FObjectImport*> RootImports;

	FString CompositeDataPath;
	FString CompositeSourcePath;

	// Cached netIndices for faster netIndex lookup. Containes only loaded objects!
	std::map<NET_INDEX, UObject*> NetIndexMap;
	// Name to Object map for faster import lookup
	std::unordered_map<FString, std::vector<FObjectExport*>> ObjectNameToExportMap;
	// List of packages we rely on
	std::mutex ExternalPackagesMutex;
	std::vector<std::shared_ptr<FPackage>> ExternalPackages;

	static FString RootDir;
	static std::recursive_mutex PackagesMutex;
	static std::vector<std::shared_ptr<FPackage>> LoadedPackages;
	static std::vector<std::shared_ptr<FPackage>> DefaultClassPackages;
	static std::vector<FString> DirCache;
	static std::unordered_map<FString, FString> PkgMap;
	static std::unordered_map<FString, FString> ObjectRedirectorMap;
	static std::unordered_map<FString, FCompositePackageMapEntry> CompositPackageMap;
	static std::unordered_map<FString, FBulkDataInfo> BulkDataMap;
	static std::unordered_map<FString, FTextureFileCacheInfo> TextureCacheMap;
	static std::mutex ClassMapMutex;
	static std::unordered_map<FString, UObject*> ClassMap;
	static std::unordered_set<FString> MissingClasses;
	static std::mutex MissingPackagesMutex;
	static std::vector<FString> MissingPackages;

	static uint16 CoreVersion;
};