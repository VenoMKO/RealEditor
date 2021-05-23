#pragma once
#include "Core.h"
#include "FName.h"
#include "FStructs.h"
#include "FStream.h"

#include <memory>
#include <atomic>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <unordered_set>

struct PackageSaveContext {
	std::string Path;
	
	ECompressionFlags Compression = COMPRESS_None;
	bool EmbedObjectPath = true;
	bool PreserveOffsets = true;
	bool DisableTextureCaching = true;
	bool FullRecook = false;

	std::string Error;

	std::function<void(int)> ProgressCallback;
	std::function<void(int)> MaxProgressCallback;
	std::function<void(std::string)> ProgressDescriptionCallback;
	std::function<bool(void)> IsCancelledCallback;
};

class FPackage : public std::enable_shared_from_this<FPackage> {
public:
	// Removes all files in a tmp dir. Should be called before any package is open.
	static void CleanCacheDir();
	// RootDir's Core.u version
	static uint16 GetCoreVersion();
	// Load Cooked Persistent Data
	static void LoadPersistentData();
	// Load class packages
	static void LoadClassPackage(const FString& name);
	// Unload class packages
	static void UnloadDefaultClassPackages();
	// Load Package Map
	static void LoadPkgMapper(bool rebuild = false);
	// Load Composite Package Map
	static void LoadCompositePackageMapper(bool rebuild = false);
	// Load ObjectReference Map
	static void LoadObjectRedirectorMapper(bool rebuild = false);
	// Load and retain a package at the path. Every GetPackage call must pair a UnloadPackage call
	static std::shared_ptr<FPackage> GetPackage(const FString& path);
	// Load and retain a package by name and guid(if valid). Every GetPackageNamed call must pair a UnloadPackage call
	static std::shared_ptr<FPackage> GetPackageNamed(const FString& name, FGuid guid = FGuid());
	// Release a package. 
	static void UnloadPackage(std::shared_ptr<FPackage> package);
	// Check if the package exists
	static bool NamedPackageExists(const FString& name, bool updateDirCache = false);
	// Find and cache all packages
	static void SetRootPath(const FString& path);
	// Set global meta data
	static void SetMetaData(const std::unordered_map<FString, std::unordered_map<FString, AMetaDataEntry>>& meta);
	// Get bulk data info
	static FBulkDataInfo* GetBulkDataInfo(const FString& bulkDataName);
	// Get texture file cache path with name
	static FString GetTextureFileCachePath(const FString& tfcName);
	// Get composite map
	static const std::unordered_map<FString, FCompositePackageMapEntry>& GetCompositePackageMap();
	// Get list of all composite packages
	static const std::unordered_map<FString, std::vector<FString>>& GetCompositePackageList();
	// Get composite package map .dat path
	static FString GetCompositePackageMapPath();
	// Get composite package name for an object path
	static FString GetObjectCompositePath(const FString& path);
	// Update DirCache
	static void UpdateDirCache();
	// Create a composite mod package
	static void CreateCompositeMod(const std::vector<FString>& items, const FString& destination, FString name, FString author);
	// Get all classes
	static std::vector<UClass*> GetClasses();
	// Register a built-in class
	static void RegisterClass(UClass* classObject);
	// Fill classes Inheritance vectors
	static void BuildClassInheritance();
	// Get the shared transaction stream
	static MTransStream& GetTransactionStream();

private:
	// Packages must be loaded/created from the static methods
	FPackage(FPackageSummary& sum)
		: Summary(sum)
	{}

public:
	~FPackage();

	// Create a ref of the package.
	// Warning! This method bypasses the FPackage retain/release mechanics.
	// Shouldn't be used unless absolutely necessary.
	std::shared_ptr<FPackage> Ref()
	{
		return shared_from_this();
	}

	// Create a read stream using DataPath and serialize tables
	void Load();

	bool Save(PackageSaveContext& options);

	// Get an object at index
	UObject* GetObject(PACKAGE_INDEX index, bool load = true);

	// Import an object from a different package.
	UObject* GetObject(FObjectImport* imp, bool load = true);
	
	// Get an object
	UObject* GetObject(FObjectExport* exp, bool load = true);

	// Get an object by path
	UObject* GetObject(const FString& path);

	// Get package index of the object. Accepts imported objects
	PACKAGE_INDEX GetObjectIndex(UObject* object) const;

	// Get an index for FName instance. The insert flag adds the name if necessary.
	PACKAGE_INDEX GetNameIndex(const FString& name, bool insert = false);

	// Get a UClass with idx
	UClass* LoadClass(PACKAGE_INDEX index);

	// Get a UClass by name
	UClass* GetClass(const FString& className);

	// Add an import object for a UClass
	PACKAGE_INDEX ImportClass(UClass* cls);

	// Cache UObject with a netIndex to NetIndexMap
	void AddNetObject(UObject* object);

	// Get an object by net index and name
	UObject* GetObject(NET_INDEX netIndex, const FString& name, const FString& className);

	// Get all exports that match the name
	std::vector<FObjectExport*> GetExportObject(const FString& name);

	// Package has changes
	inline bool IsDirty() const
	{
		return Summary.PackageFlags & PKG_Dirty;
	}

	// Mark package as dirty/clean
	void MarkDirty(bool dirty = true);

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

	// Check if the package has the provided flag
	inline bool GetPackageFlag(EPackageFlags flag)
	{
		return Summary.PackageFlags & flag;
	}

	// Toggle the provided flag
	inline void SetPackageFlag(EPackageFlags flag, bool state = true)
	{
		if (state)
		{
			Summary.PackageFlags |= flag;
		}
		else
		{
			Summary.PackageFlags &= ~flag;
		}
	}

	// Get import/export object at index
	inline FObjectResource* GetResourceObject(PACKAGE_INDEX index) const
	{
		if (index < 0)
		{
			return (FObjectResource*)GetImportObject(index);
		}
		if (index > 0)
		{
			return (FObjectResource*)GetExportObject(index);
		}
		return nullptr;
	}

	// Get import object at index
	inline FObjectImport* GetImportObject(PACKAGE_INDEX index) const
	{
		const PACKAGE_INDEX i = -index - 1;
		return Imports[i];
	}

	// Find an existing import object with the exact name and class
	FObjectImport* GetImportObject(const FString& objectName, const FString& className) const;

	// Get export object at index
	inline FObjectExport* GetExportObject(PACKAGE_INDEX index) const
	{
		const PACKAGE_INDEX i = index - 1;
		return Exports[i];
	}

	// Add an object to package imports. Returns true if added
	bool AddImport(UObject* object, FObjectImport*& output);

	// Add an export to the exports table and create a UObject
	FObjectExport* AddExport(const FString& objectName, const FString& objectClass, FObjectExport* parent);

	// Duplicate the source object and move it to the dest's inner. If dest is null, ROOT_EXPORT will be used.
	FObjectExport* DuplicateExport(FObjectExport* source, FObjectExport* dest, const FString& objectName);

	// Remove an export added by AddExport. Invalidates exp pointer!!!
	void RemoveExport(FObjectExport* exp);

	// Convert source object to a UObjectRedirector pointing to the target object
	// Dangerous!!!
	// After this call all old pointers to the source object will point to a freed region of memory!!!
	void ConvertObjectToRedirector(UObject*& source, UObject* targer);

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

	// Get package size. If package was decompressed, return compressed size!
	inline size_t GetSourceSize() const
	{
		return Summary.SourceSize;
	}

	// Get name at index
	inline FString GetIndexedName(NAME_INDEX index) const
	{
		return Names[index].GetString();
	}

	// Get Summary's FolderName
	inline FString GetFolderName() const
	{
		return Summary.FolderName;
	}

	// Modify Summary's FolderName
	inline void SetFolderName(const FString& name)
	{
		if (Summary.FolderName != name)
		{
			MarkDirty();
		}
		Summary.FolderName = name;
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

	// Get package's data path(may differ from a SourcePath)
	inline FString GetDataPath() const
	{
		return Summary.DataPath;
	}

	// Get package's composite storage gpk
	inline FString GetCompositeSourcePath() const
	{
		return CompositeSourcePath;
	}

	// Returns true if Load() finished
	inline bool IsReady() const
	{
		return Ready.load();
	}

	// Cancel loading operation
	void CancelOperation()
	{
		Cancelled.store(true);
	}

	// Check if the load operation was canceled
	bool IsOperationCancelled() const
	{
		return Cancelled.load();
	}

	// Package file/archive version
	inline uint16 GetFileVersion() const
	{
		return Summary.GetFileVersion();
	}

	// Package licensee version
	inline uint16 GetLicenseeVersion() const
	{
		return Summary.GetLicenseeVersion();
	}

	// Package header
	inline FPackageSummary GetSummary() const
	{
		return Summary;
	}

	// Table of texture allocations. Writable!
	inline FTextureAllocations& GetTextureAllocations()
	{
		return Summary.TextureAllocations;
	}

	// Get package composite path aka object path(i.e. 1af4203b10/Skel/Test_dup) or an empty string
	FString GetCompositePath();
	
	// Used to create built-in classes
	VObjectExport* CreateVirtualExport(const char* objName, const char* clsName);

	// Get package name
	FString GetPackageName(bool extension = false) const;

	// Get all package name entries
	inline std::vector<FString> GetNames() const
	{
		std::vector<FString> result;
		for (const FNameEntry& name : Names)
		{
			result.push_back(name.GetString());
		}
		return result;
	}

	inline bool IsReadOnly() const
	{
		return !AllowEdit || Composite;
	}

	inline bool IsComposite() const
	{
		return Composite;
	}

	// Add a package to a temporary depends list so it won't get released while the current package still needs it.
	void RetainPackage(std::shared_ptr<FPackage> package);

private:
	void _DebugDump() const;

	UObject* GetCachedExportObject(PACKAGE_INDEX index) const;
	UObject* GetCachedForcedObject(PACKAGE_INDEX index) const;
	UObject* GetCachedImportObject(PACKAGE_INDEX index) const;

	UObject* SetCachedExportObject(PACKAGE_INDEX index, UObject* obj);
	UObject* SetCachedForcedObject(PACKAGE_INDEX index, UObject* obj);
	UObject* SetCachedImportObject(PACKAGE_INDEX index, UObject* obj);

	UObject* GetForcedExport(FObjectExport* exp);

	FObjectExport* DuplicateExportRecursivly(FObjectExport* source, FObjectExport* dest, const FString& objectName);

private:
	FPackageSummary Summary;
	FStream* Stream = nullptr;
	class UObjectReferencer* Referencer = nullptr;

	// Prevent multiple Load() calls
	std::atomic_bool Loading = { false };
	// Load finished 
	std::atomic_bool Ready = { false };
	// Load was canceled
	std::atomic_bool Cancelled = { false };

	bool AllowEdit = true;
	bool Composite = false;
	bool AllowForcedExportResolving = true;

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

	std::vector<FLevelGuids> ImportGuids;
	std::map<FGuid, FObjectExport*>	ExportGuids;

	std::vector<FObjectThumbnailInfo*> ThumbnailInfos;
	std::vector<FObjectThumbnail*> Thumbnails;

	std::vector<FObjectExport*> RootExports;
	std::vector<FObjectImport*> RootImports;

	FString CompositeDataPath;
	FString CompositeSourcePath;

	// Cached netIndices for faster netIndex lookup. Contains only loaded objects!
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
	static std::unordered_map<FString, FString> TfcCache;
	static std::unordered_map<FString, FString> PkgMap;
	static std::unordered_map<FString, FString> ObjectRedirectorMap;
	static std::unordered_map<FString, FCompositePackageMapEntry> CompositPackageMap;
	static std::unordered_map<FString, std::vector<FString>> CompositPackageList;
	static std::unordered_map<FString, FBulkDataInfo> BulkDataMap;
	static std::unordered_map<FString, FTextureFileCacheInfo> TextureCacheMap;
	static std::unordered_map<FString, std::unordered_map<FString, AMetaDataEntry>> MetaData;
	static std::mutex ClassMapMutex;
	static std::unordered_map<FString, UObject*> ClassMap;
	static std::unordered_set<FString> MissingClasses;
	static std::mutex MissingPackagesMutex;
	static std::vector<FString> MissingPackages;
	static MTransStream TranscationStream;

	static uint16 CoreVersion;
public:
	static std::vector<FString> FilePackageNames;
};