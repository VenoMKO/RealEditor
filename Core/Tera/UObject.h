#pragma once
#include "Core.h"
#include "FStream.h"
#include "FPropertyTag.h"
#include "UObjectHelpers.h"

class UObject {
public:
  // =========================================================================
  // Specialized methods used by DECL_UOBJ, ClassCast, etc. Don't override!!!
  // =========================================================================

  enum { StaticClassCastFlags = CASTCLASS_None };
  virtual uint32 GetStaticClassCastFlags() const
  {
    return StaticClassCastFlags;
  }

  // Cpp class name. Use ONLY with a class (e.g., UObject::StaticClass())
  static const char* StaticClassName()
  {
    return "Object";
  }

  // Get cpp class name from an instance
  virtual const char* GetStaticClassName() const
  {
    return StaticClassName();
  }

  // Internal method used for safe casting
  virtual std::string GetStaticClassChain() const
  {
    return StaticClassName();
  }

  // =========================================================================
  // Normal UObject methods
  // =========================================================================

  // Helper for loading nonull objects
  static void LoadObject(UObject* object)
  {
    if (object)
    {
      object->Load();
    }
  }

  // Object factory
  static UObject* Object(FObjectExport* exp);

  UObject() = delete;
  UObject(const UObject&) = delete;
  UObject(FObjectExport* exp);

  virtual ~UObject();

  // Load the object from its package. Creates a copy of package's stream
  virtual void Load();

  // Load the object from the provided stream. Will set correct stream offset.
  virtual void Load(FStream& s);

  // Serialize object by an index
  friend FStream& operator<<(FStream& s, UObject*& obj);

  // Used to collect properties during serialization.
  virtual bool RegisterProperty(FPropertyTag* property);

  FString GetObjectPath() const;

  FName GetObjectName() const;

  FString GetObjectNameString() const;

  FName GetClassName() const;

  FString GetClassNameString() const;

  uint32 GetExportFlags() const;

  void SetExportFlags(uint32 flags);

  uint64 GetObjectFlags() const;

  void SetObjectFlags(uint64 flags);

  FString GetFullObjectName() const;

  inline FObjectExport* GetExportObject() const
  {
    return Export;
  }

  inline NET_INDEX GetNetIndex() const
  {
    return NetIndex;
  }

  inline void SetNetIndex(NET_INDEX idx)
  {
    NetIndex = idx;
  }

  FPackage* GetPackage() const;

  // Is UComponent?
  virtual bool IsComponent() const
  {
    return false;
  }

  inline bool IsTransacting() const
  {
    return HasAnyFlags(RF_IsTransacting);
  }

  void SetTransacting(bool flag);

  inline bool IsNewTrans() const
  {
    return HasAnyFlags(RF_TransNew);
  }

  void SetIsNewTrans(bool flag);

  // Check if UObject has any object flags
  bool HasAnyFlags(uint64 flags) const;

  UObject* GetOuter() const
  {
    return Outer;
  }

  std::vector<UObject*> GetInner() const
  {
    return Inner;
  }

  void MarkDirty(bool dirty = true);

  inline bool IsDirty() const
  {
    return HasAnyFlags(RF_Marked);
  }

  FILE_OFFSET GetSerialOffset() const;
  
  FILE_OFFSET GetSerialSize() const;

  FILE_OFFSET GetPropertiesSize() const;

  FILE_OFFSET GetDataSize() const;

  bool IsTemplate(uint64 templateTypes = (RF_ArchetypeObject | RF_ClassDefaultObject)) const
  {
    for (const UObject* outer = this; outer; outer = outer->GetOuter())
    {
      if (outer->HasAnyFlags((EObjectFlags)templateTypes))
      {
        return true;
      }
    }
    return false;
  }

  template<typename T>
  T* GetTypedOuter() const
  {
    T* result = NULL;
    for (UObject* nextOuter = Outer; result == NULL && nextOuter != NULL; nextOuter = nextOuter->GetOuter())
    {
      if (nextOuter->IsA(T::StaticClassName()))
      {
        result = (T*)nextOuter;
      }
    }
    return result;
  }

  bool IsA(const char* base) const;

  inline bool IsLoaded() const
  {
    return Loaded;
  }

  inline void SetLoaded(bool loaded)
  {
    Loaded = loaded;
  }

  // Get object's class object
  inline UClass* GetClass() const
  {
    return Class;
  }

  inline void SetClass(UClass* cls)
  {
    Class = cls;
  }

  // Get object's root properties
  std::vector<FPropertyTag*> GetProperties() const
  {
    return Properties;
  }

  // Create a new property. Allocates properties value. Doesn't add the property to objects Properties!
  virtual FPropertyTag* CreateProperty(const FString& name);

  // Add root property to the object. Don't call this with a struct/array element property!
  void AddProperty(FPropertyTag* property);

  // Remove root property
  void RemoveProperty(FPropertyTag* tag);
  
  // Object initialization. Wont modify package's object tree!
  void SetOuter(UObject* outer)
  {
    Outer = outer;
  }

  // Object initialization. Wont modify package's object tree!
  void AddInner(UObject* inner)
  {
    Inner.push_back(inner);
  }

  // Get objects raw binary data. The data is unmodified unless SetRawData was called.
  void* GetRawData();

  // Set raw binary data
  void SetRawData(void* data, FILE_OFFSET size);

  // Serialize the object from a stream. Should not be called outside of the Load
  virtual void Serialize(FStream& s);

  // Create a filepath to the object with a given separator
  FString GetLocalDir(bool fileName = false, const char* sep = "\\");

private:
  virtual void SerializeScriptProperties(FStream& s);

protected:
  // Handle object initialization for derived classes
  virtual void PostLoad();

  // Special serialization for class default objects
  void SerializeDefaultObject(FStream& s);

  // Helper to serialize object data leftovers(e.g., incomplete implementation). Should be called in the Serialize method of a derived class
  void SerializeTrailingData(FStream& s);

protected:
  bool Loaded = false;
  FObjectExport* Export = nullptr;
  FStateFrame* StateFrame = nullptr;
  NET_INDEX NetIndex = INDEX_NONE;

  UClass* Class = nullptr;
  UObject* DefaultObject = nullptr;

  UObject* Outer = nullptr;
  std::vector<UObject*> Inner;

  std::vector<FPropertyTag*> Properties;

  FILE_OFFSET RawDataOffset = 0;
  FILE_OFFSET RawDataSize = 0;
  char* RawData = nullptr;

  void* TrailingData = nullptr;
  FILE_OFFSET TrailingDataSize = 0;
#ifdef _DEBUG
  std::string Description;
#endif
private:
  bool Loading = false;
};

class UPackage : public UObject {
public:
  DECL_UOBJ(UPackage, UObject);
};