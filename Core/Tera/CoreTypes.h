#pragma once
using int8 = signed char;
using uint8 = unsigned char;

using int16 = signed short int;
using uint16 = unsigned short int;

using int32 = signed int;
using uint32 = unsigned int;

using int64 = signed long long;
using uint64 = unsigned long long;

using ubool = uint32;

using wchar = wchar_t;

#include "FFlags.h"
#include "UNames.h"

using FILE_OFFSET = int32;
using NAME_INDEX = int32;
using PACKAGE_INDEX = int32;
using NET_INDEX = int32;
using BITFIELD = unsigned long;

enum { INDEX_NONE = -1 };
enum { MAX_TEXCOORDS = 4 };
enum { MAX_GPUSKIN_BONES = 75 };
enum { MSP_MAX = 1 };

#define PACKAGE_MAGIC 0x9E2A83C1

#include <string>
// Use std::vector instead of TArray
#include <vector>
// Use std::map instead of TMap
#include <map>

class FName;
class FStream;
class FPackage;
class FString;
class FStateFrame;
class FObjectResource;
class FObjectImport;
class FObjectExport;
class VObjectExport;

class UObject;
class UComponent;
class UField;
class UConst;
class UEnum;
class UProperty;
class UByteProperty;
class UIntProperty;
class UBoolProperty;
class UFloatProperty;
class UObjectProperty;
class UComponentProperty;
class UClassProperty;
class UInterfaceProperty;
class UNameProperty;
class UStructProperty;
class UStrProperty;
class UArrayProperty;
class UMapProperty;
class UDelegateProperty;
class UStruct;
class UFunction;
class UState;
class UClass;
class UScriptStruct;
class UTextBuffer;
class UObjectRedirector;