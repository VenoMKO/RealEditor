#pragma once

enum EPackageFlags : uint32
{
  PKG_AllowDownload = 0x00000001,	// Allow downloading package.
  PKG_ClientOptional = 0x00000002,	// Purely optional for clients.
  PKG_ServerSideOnly = 0x00000004,   // Only needed on the server side.
  PKG_Cooked = 0x00000008,	// Whether this package has been cooked for the target platform.
  PKG_Unsecure = 0x00000010,   // Not trusted.
  PKG_SavedWithNewerVersion = 0x00000020,	// Package was saved with newer version.
  PKG_NoSource = 0x00000040,
  PKG_ROAccess = 0x00000080,
  PKG_Dirty = 0x00000100,
  PKG_PendingDeletion = 0x00000200,
  PKG_Need = 0x00008000,	// Client needs to download this package.
  PKG_Compiling = 0x00010000,	// package is currently being compiled
  PKG_ContainsMap = 0x00020000,	// Set if the package contains a ULevel/ UWorld object
  PKG_Trash = 0x00040000,	// Set if the package was loaded from the trashcan
  PKG_DisallowLazyLoading = 0x00080000,	// Set if the archive serializing this package cannot use lazy loading
  PKG_PlayInEditor = 0x00100000,	// Set if the package was created for the purpose of PIE
  PKG_ContainsScript = 0x00200000,	// Package is allowed to contain UClasses and unrealscript
  PKG_ContainsDebugInfo = 0x00400000,	// Package contains debug info (for UDebugger)
  PKG_RequireImportsAlreadyLoaded = 0x00800000,	// Package requires all its imports to already have been loaded
  PKG_SelfContainedLighting = 0x01000000,	// All lighting in this package should be self contained
  PKG_StoreCompressed = 0x02000000,	// Package is being stored compressed, requires archive support for compression
  PKG_StoreFullyCompressed = 0x04000000,	// Package is serialized normally, and then fully compressed after (must be decompressed before LoadPackage is called)
  PKG_ContainsInlinedShaders = 0x08000000,	// Package was cooked allowing materials to inline their FMaterials (and hence shaders)
  PKG_ContainsFaceFXData = 0x10000000,	// Package contains FaceFX assets and/or animsets
  PKG_NoExportAllowed = 0x20000000,	// Package was NOT created by a modder.  Internal data not for export
  PKG_StrippedSource = 0x40000000,	// Source has been removed to compress the package size
};

enum EObjectFlags : uint64
{
  RF_InSingularFunc = 0x0000000000000002,		// In a singular function.
  RF_StateChanged = 0x0000000000000004,		// Object did a state change.
  RF_DebugPostLoad = 0x0000000000000008,		// For debugging PostLoad calls.
  RF_DebugSerialize = 0x0000000000000010,		// For debugging Serialize calls.
  RF_DebugFinishDestroyed = 0x0000000000000020,		// For debugging FinishDestroy calls.
  RF_EdSelected = 0x0000000000000040,		// Object is selected in one of the editors browser windows.
  RF_ZombieComponent = 0x0000000000000080,		// This component's template was deleted, so should not be used.
  RF_Protected = 0x0000000000000100,		// Property is protected (may only be accessed from its owner class or subclasses)
  RF_ClassDefaultObject = 0x0000000000000200,		// this object is its class's default object
  RF_ArchetypeObject = 0x0000000000000400,		// this object is a template for another object - treat like a class default object
  RF_ForceTagExp = 0x0000000000000800,		// Forces this object to be put into the export table when saving a package regardless of outer
  RF_TokenStreamAssembled = 0x0000000000001000,		// Set if reference token stream has already been assembled
  RF_MisalignedObject = 0x0000000000002000,		// Object's size no longer matches the size of its C++ class (only used during make, for native classes whose properties have changed)
  RF_RootSet = 0x0000000000004000,		// Object will not be garbage collected, even if unreferenced.
  RF_BeginDestroyed = 0x0000000000008000,		// BeginDestroy has been called on the object.
  RF_FinishDestroyed = 0x0000000000010000,		// FinishDestroy has been called on the object.
  RF_DebugBeginDestroyed = 0x0000000000020000,		// Whether object is rooted as being part of the root set (garbage collection)
  RF_MarkedByCooker = 0x0000000000040000,		// Marked by content cooker.
  RF_LocalizedResource = 0x0000000000080000,		// Whether resource object is localized.
  RF_InitializedProps = 0x0000000000100000,		// whether InitProperties has been called on this object
  RF_PendingFieldPatches = 0x0000000000200000,		//@script patcher: indicates that this struct will receive additional member properties from the script patcher
  RF_IsCrossLevelReferenced = 0x0000000000400000,		// This object has been pointed to by a cross-level reference, and therefore requires additional cleanup upon deletion
  RF_IsTransacting = 0x0000000000800000, // The object is being moved
  RF_TransNew = 0x0000000001000000, // The object is new and needs serialization(used by MTransStream to prevent double-serialization of existing objects)
  RF_ObjectIsRO = 0x0000000002000000,

  RF_Saved = 0x0000000080000000,		// Object has been saved via SavePackage. Temporary.
  RF_Transactional = 0x0000000100000000,		// Object is transactional.
  RF_Unreachable = 0x0000000200000000,		// Object is not reachable on the object graph.
  RF_Public = 0x0000000400000000,		// Object is visible outside its package.
  RF_TagImp = 0x0000000800000000,		// Temporary import tag in load/save.
  RF_TagExp = 0x0000001000000000,		// Temporary export tag in load/save.
  RF_Obsolete = 0x0000002000000000,		// Object marked as obsolete and should be replaced.
  RF_TagGarbage = 0x0000004000000000,		// Check during garbage collection.
  RF_DisregardForGC = 0x0000008000000000,		// Object is being disregard for GC as its static and itself and all references are always loaded.
  RF_PerObjectLocalized = 0x0000010000000000,		// Object is localized by instance name, not by class.
  RF_NeedLoad = 0x0000020000000000,		// During load, indicates object needs loading.
  RF_AsyncLoading = 0x0000040000000000,		// Object is being asynchronously loaded.
  RF_NeedPostLoadSubobjects = 0x0000080000000000,		// During load, indicates that the object still needs to instance subobjects and fixup serialized component references
  RF_Suppress = 0x0000100000000000,		// @warning: Mirrored in UnName.h. Suppressed log name.
  RF_InEndState = 0x0000200000000000,		// Within an EndState call.
  RF_Transient = 0x0000400000000000,		// Don't save object.
  RF_Cooked = 0x0000800000000000,		// Whether the object has already been cooked
  RF_LoadForClient = 0x0001000000000000,		// In-file load for client.
  RF_LoadForServer = 0x0002000000000000,		// In-file load for client.
  RF_LoadForEdit = 0x0004000000000000,		// In-file load for client.
  RF_Standalone = 0x0008000000000000,		// Keep object around for editing even if unreferenced.
  RF_NotForClient = 0x0010000000000000,		// Don't load this object for the game client.
  RF_NotForServer = 0x0020000000000000,		// Don't load this object for the game server.
  RF_NotForEdit = 0x0040000000000000,		// Don't load this object for the editor.
  RF_NeedPostLoad = 0x0100000000000000,		// Object needs to be postloaded.
  RF_HasStack = 0x0200000000000000,		// Has execution stack.
  RF_Native = 0x0400000000000000,		// Native (UClass only).
  RF_Marked = 0x0800000000000000,		// Marked (for debugging).
  RF_ErrorShutdown = 0x1000000000000000,		// ShutdownAfterError called.
  RF_PendingKill = 0x2000000000000000,		// Objects that are pending destruction (invalid for gameplay but valid objects)
  RF_AllFlags = 0xFFFFFFFFFFFFFFFF
};

enum EFExportFlags : uint32
{
  EF_None = 0x00000000, // No flags
  EF_ForcedExport = 0x00000001, // Whether the export was forced into the export table via RF_ForceTagExp.
  EF_ScriptPatcherExport = 0x00000002, // indicates that this export was added by the script patcher, so this object's data will come from memory, not disk
  EF_MemberFieldPatchPending = 0x00000004, // indicates that this export is a UStruct which will be patched with additional member fields by the script patcher
  EF_Composit = 0x00000008
};

enum AnimationKeyFormat : uint8 {
  AKF_ConstantKeyLerp = 0,
  AKF_VariableKeyLerp = 1,
  AKF_PerTrackCompression = 2,
  AKF_MAX = 3,
};

enum EPixelFormat : uint32
{
  PF_Unknown = 0,
  PF_A32B32G32R32F = 1,
  PF_A8R8G8B8 = 2,
  PF_G8 = 3,
  PF_G16 = 4,
  PF_DXT1 = 5,
  PF_DXT3 = 6,
  PF_DXT5 = 7,
  PF_UYVY = 8,
  PF_FloatRGB = 9,
  PF_FloatRGBA = 10,
  PF_DepthStencil = 11,
  PF_ShadowDepth = 12,
  PF_FilteredShadowDepth = 13,
  PF_R32F = 14,
  PF_G16R16 = 15,
  PF_G16R16F = 16,
  PF_G16R16F_FILTER = 17,
  PF_G32R32F = 18,
  PF_A2B10G10R10 = 19,
  PF_A16B16G16R16 = 20,
  PF_D24 = 21,
  PF_R16F = 22,
  PF_R16F_FILTER = 23,
  PF_BC5 = 24,
  PF_V8U8 = 25,
  PF_A1 = 26,
  PF_FloatR11G11B10 = 27,
  PF_Max = 28
};

enum EClassFlags : uint32
{
  CLASS_None = 0x00000000,
  CLASS_Abstract = 0x00000001,
  CLASS_Compiled = 0x00000002,
  CLASS_Config = 0x00000004,
  CLASS_Transient = 0x00000008,
  CLASS_Parsed = 0x00000010,
  CLASS_Localized = 0x00000020,
  CLASS_SafeReplace = 0x00000040,
  CLASS_Native = 0x00000080,
  CLASS_NoExport = 0x00000100,
  CLASS_Placeable = 0x00000200,
  CLASS_PerObjectConfig = 0x00000400,
  CLASS_NativeReplication = 0x00000800,
  CLASS_EditInlineNew = 0x00001000,
  CLASS_CollapseCategories = 0x00002000,
  CLASS_Interface = 0x00004000,
  CLASS_HasInstancedProps = 0x00200000,
  CLASS_NeedsDefProps = 0x00400000,
  CLASS_HasComponents = 0x00800000,
  CLASS_Hidden = 0x01000000,
  CLASS_Deprecated = 0x02000000,
  CLASS_HideDropDown = 0x04000000,
  CLASS_Exported = 0x08000000,
  CLASS_Intrinsic = 0x10000000,
  CLASS_NativeOnly = 0x20000000,
  CLASS_PerObjectLocalized = 0x40000000,
  CLASS_HasCrossLevelRefs = 0x80000000,
  CLASS_IsAUProperty = 0x00008000,
  CLASS_IsAUObjectProperty = 0x00010000,
  CLASS_IsAUBoolProperty = 0x00020000,
  CLASS_IsAUState = 0x00040000,
  CLASS_IsAUFunction = 0x00080000,
  CLASS_IsAUStructProperty = 0x00100000,
  CLASS_AllFlags = 0xFFFFFFFF
};

enum EClassCastFlag : uint32
{
  CASTCLASS_None = 0x00000000,
  CASTCLASS_UField = 0x00000001,
  CASTCLASS_UConst = 0x00000002,
  CASTCLASS_UEnum = 0x00000004,
  CASTCLASS_UStruct = 0x00000008,
  CASTCLASS_UScriptStruct = 0x00000010,
  CASTCLASS_UClass = 0x00000020,
  CASTCLASS_UByteProperty = 0x00000040,
  CASTCLASS_UIntProperty = 0x00000080,
  CASTCLASS_UFloatProperty = 0x00000100,
  CASTCLASS_UComponentProperty = 0x00000200,
  CASTCLASS_UClassProperty = 0x00000400,
  CASTCLASS_UInterfaceProperty = 0x00001000,
  CASTCLASS_UNameProperty = 0x00002000,
  CASTCLASS_UStrProperty = 0x00004000,
  CASTCLASS_UProperty = 0x00008000,
  CASTCLASS_UObjectProperty = 0x00010000,
  CASTCLASS_UBoolProperty = 0x00020000,
  CASTCLASS_UState = 0x00040000,
  CASTCLASS_UFunction = 0x00080000,
  CASTCLASS_UStructProperty = 0x00100000,
  CASTCLASS_UArrayProperty = 0x00200000,
  CASTCLASS_UMapProperty = 0x00400000,
  CASTCLASS_UDelegateProperty = 0x00800000,
  CASTCLASS_UComponent = 0x01000000,
  CASTCLASS_AllFlags = 0xFFFFFFFF,
};

enum : uint64 {
  CPF_Edit = 0x0000000000000001,
  CPF_Const = 0x0000000000000002,
  CPF_Input = 0x0000000000000004,
  CPF_ExportObject = 0x0000000000000008,
  CPF_OptionalParm = 0x0000000000000010,
  CPF_Net = 0x0000000000000020,
  CPF_EditFixedSize = 0x0000000000000040,
  CPF_Parm = 0x0000000000000080,
  CPF_OutParm = 0x0000000000000100,
  CPF_SkipParm = 0x0000000000000200,
  CPF_ReturnParm = 0x0000000000000400,
  CPF_CoerceParm = 0x0000000000000800,
  CPF_Native = 0x0000000000001000,
  CPF_Transient = 0x0000000000002000,
  CPF_Config = 0x0000000000004000,
  CPF_Localized = 0x0000000000008000,

  CPF_EditConst = 0x0000000000020000,
  CPF_GlobalConfig = 0x0000000000040000,
  CPF_Component = 0x0000000000080000,
  CPF_AlwaysInit = 0x0000000000100000,
  CPF_DuplicateTransient = 0x0000000000200000,
  CPF_NeedCtorLink = 0x0000000000400000,
  CPF_NoExport = 0x0000000000800000,
  CPF_NoImport = 0x0000000001000000,
  CPF_NoClear = 0x0000000002000000,
  CPF_EditInline = 0x0000000004000000,

  CPF_EditInlineUse = 0x0000000010000000,
  CPF_Deprecated = 0x0000000020000000,
  CPF_DataBinding = 0x0000000040000000,
  CPF_SerializeText = 0x0000000080000000,

  CPF_RepNotify = 0x0000000100000000,
  CPF_Interp = 0x0000000200000000,
  CPF_NonTransactional = 0x0000000400000000,

  CPF_EditorOnly = 0x0000000800000000,
  CPF_NotForConsole = 0x0000001000000000,
  CPF_RepRetry = 0x0000002000000000,
  CPF_PrivateWrite = 0x0000004000000000,
  CPF_ProtectedWrite = 0x0000008000000000,
  CPF_ArchetypeProperty = 0x0000010000000000,

  CPF_EditHide = 0x0000020000000000,
  CPF_EditTextBox = 0x0000040000000000,

  CPF_CrossLevelPassive = 0x0000100000000000,
  CPF_CrossLevelActive = 0x0000200000000000,
};

enum ECompressionFlags
{
  COMPRESS_None = 0x00,
  COMPRESS_ZLIB = 0x01,
  COMPRESS_LZO = 0x02,
  COMPRESS_LZX = 0x04
};

enum EBulkDataFlags
{
  BULKDATA_None = 0,
  BULKDATA_StoreInSeparateFile = 1 << 0,
  BULKDATA_SerializeCompressedZLIB = 1 << 1,
  BULKDATA_ForceSingleElementSerialization = 1 << 2,
  BULKDATA_SingleUse = 1 << 3,
  BULKDATA_SerializeCompressedLZO = 1 << 4,
  BULKDATA_Unused = 1 << 5,
  BULKDATA_StoreOnlyPayload = 1 << 6,
  BULKDATA_SerializeCompressedLZX = 1 << 7,
  BULKDATA_SerializeCompressed = (BULKDATA_SerializeCompressedZLIB | BULKDATA_SerializeCompressedLZO | BULKDATA_SerializeCompressedLZX),
};

enum TextureAddress : uint8
{
  TA_Wrap = 0,
  TA_Clamp = 1,
  TA_Mirror = 2,
  TA_MAX = 3,
};

enum TextureGroup : uint8
{
  TEXTUREGROUP_World = 0,
  TEXTUREGROUP_WorldNormalMap = 1,
  TEXTUREGROUP_WorldSpecular = 2,
  TEXTUREGROUP_Character = 3,
  TEXTUREGROUP_CharacterNormalMap = 4,
  TEXTUREGROUP_CharacterSpecular = 5,
  TEXTUREGROUP_Weapon = 6,
  TEXTUREGROUP_WeaponNormalMap = 7,
  TEXTUREGROUP_WeaponSpecular = 8,
  TEXTUREGROUP_Vehicle = 9,
  TEXTUREGROUP_VehicleNormalMap = 10,
  TEXTUREGROUP_VehicleSpecular = 11,
  TEXTUREGROUP_Cinematic = 12,
  TEXTUREGROUP_Effects = 13,
  TEXTUREGROUP_EffectsNotFiltered = 14,
  TEXTUREGROUP_Skybox = 15,
  TEXTUREGROUP_UI = 16,
  TEXTUREGROUP_Lightmap = 17,
  TEXTUREGROUP_RenderTarget = 18,
  TEXTUREGROUP_MobileFlattened = 19,
  TEXTUREGROUP_ProcBuilding_Face = 20,
  TEXTUREGROUP_ProcBuilding_LightMap = 21,
  TEXTUREGROUP_Shadowmap = 22,
  TEXTUREGROUP_ColorLookupTable = 23,
  TEXTUREGROUP_Terrain_Heightmap = 24,
  TEXTUREGROUP_Terrain_Weightmap = 25,
  TEXTUREGROUP_ImageBasedReflection = 26,
  TEXTUREGROUP_Bokeh = 27,
  TEXTUREGROUP_MAX = 28,
};

enum TextureCompressionSettings : uint8
{
  TC_Default = 0,
  TC_Normalmap = 1,
  TC_Displacementmap = 2,
  TC_NormalmapAlpha = 3,
  TC_Grayscale = 4,
  TC_HighDynamicRange = 5,
  TC_OneBitAlpha = 6,
  TC_NormalmapUncompressed = 7,
  TC_NormalmapBC5 = 8,
  TC_OneBitMonochrome = 9,
  TC_SimpleLightmapModification = 10,
  TC_VectorDisplacementmap = 11,
  TC_MAX = 12,
};

enum class MipFilterType
{
  Box = 0,
  Triangle,
  Kaiser,
  Mitchell
};

enum EBlendMode
{
  BLEND_Opaque = 0,
  BLEND_Masked,
  BLEND_Translucent,
  BLEND_Additive,
  BLEND_Modulate,
  BLEND_ModulateAndAdd,
  BLEND_SoftMasked,
  BLEND_AlphaComposite,
  BLEND_DitheredTranslucent,
  BLEND_MAX,
};

enum EBlendMode32
{
  BLEND32_Opaque = 0,
  BLEND32_Masked,
  BLEND32_Translucent,
  BLEND32_Additive,
  BLEND32_Modulate,
  BLEND32_SoftMasked,
  BLEND32_AlphaComposite,
  BLEND32_DitheredTranslucent,
  BLEND32_MAX,
};

enum AnimationCompressionFormat : uint8
{
  ACF_None = 0,
  ACF_Float96NoW = 1,
  ACF_Fixed48NoW = 2,
  ACF_IntervalFixed32NoW = 3,
  ACF_Fixed32NoW = 4,
  ACF_Float32NoW = 5,
  ACF_Identity = 6,
  ACF_MAX = 7,
};

enum EMaterialLightingModel
{
  MLM_Phong,
  MLM_NonDirectional,
  MLM_Unlit,
  MLM_SHPRT,
  MLM_Custom,
  MLM_S1Phong,
  MLM_Anisotropic, // Unavailable on x86
};

enum SoundDistanceModel
{
  ATTENUATION_Linear = 0,
  ATTENUATION_Logarithmic = 1,
  ATTENUATION_Inverse = 2,
  ATTENUATION_LogReverse = 3,
  ATTENUATION_NaturalSound = 4,
  ATTENUATION_MAX = 5,
};

enum ReverbPreset
{
  REVERB_Default = 0,
  REVERB_Bathroom = 1,
  REVERB_StoneRoom = 2,
  REVERB_Auditorium = 3,
  REVERB_ConcertHall = 4,
  REVERB_Cave = 5,
  REVERB_Hallway = 6,
  REVERB_StoneCorridor = 7,
  REVERB_Alley = 8,
  REVERB_Forest = 9,
  REVERB_City = 10,
  REVERB_Mountains = 11,
  REVERB_Quarry = 12,
  REVERB_Plain = 13,
  REVERB_ParkingLot = 14,
  REVERB_SewerPipe = 15,
  REVERB_Underwater = 16,
  REVERB_SmallRoom = 17,
  REVERB_MediumRoom = 18,
  REVERB_LargeRoom = 19,
  REVERB_MediumHall = 20,
  REVERB_LargeHall = 21,
  REVERB_Plate = 22,
  REVERB_MAX = 23,
};