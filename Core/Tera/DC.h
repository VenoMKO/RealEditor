#pragma once
#include "FStream.h"
#include <pugixml/pugixml.hpp>
#include <filesystem>

namespace S1Data
{
  struct DCName {
    uint16 Index = 0;
    friend FStream& operator<<(FStream& s, DCName& n)
    {
      return s << n.Index;
    }
    inline bool operator==(const DCName& b) const
    {
      return Index == b.Index;
    }
  };

  struct DCIndex {
    uint16 IndexA = 0xFFFF;
    uint16 IndexB = 0xFFFF;
    friend FStream& operator<<(FStream& s, DCIndex& i)
    {
      return s << i.IndexA << i.IndexB;
    }
    bool operator==(const DCIndex& b) const
    {
      return IndexA == b.IndexA && IndexB == b.IndexB;
    }
  };

  struct DCHeader {
    int32 Unk1 = 0;
    int64 Unk2 = 0;
    int32 Version = 0;
    int32 Unk3 = 0;
    int32 Unk4 = 0;
    int32 Unk5 = 0;
    int32 Unk6 = 0;

    friend FStream& operator<<(FStream& s, DCHeader& h)
    {
      s << h.Unk1 << h.Unk2;
      s << h.Version;
      s << h.Unk3 << h.Unk4;
      s << h.Unk5 << h.Unk6;
      return s;
    }
  };

  struct UnkStruct {
    uint32 Unk1 = 0;
    uint32 Unk2 = 0;

    friend FStream& operator<<(FStream& s, UnkStruct& unk)
    {
      return s << unk.Unk1 << unk.Unk2;
    }
  };

  struct DCStringInfo {
    uint32 Hash = 0;
    uint32 Length = 0;
    uint32 Id = 0;
    DCIndex Indices;

    friend FStream& operator<<(FStream& s, DCStringInfo& i)
    {
      return s << i.Hash << i.Length << i.Id << i.Indices;
    }
  };

  struct DCElement64 {
    DCName Name;
    uint16 Index = 0;
    uint16 AttributeCount = 0;
    uint16 ChildrenCount = 0;
    DCIndex AttributeIndices;
    uint32 Padding1 = 0;
    DCIndex ChildrenIndices;
    uint32 Padding2 = 0;

    friend FStream& operator<<(FStream& s, DCElement64& e)
    {
      s << e.Name;
      s << e.Index;
      s << e.AttributeCount;
      s << e.ChildrenCount;
      s << e.AttributeIndices;
      s << e.Padding1;
      s << e.ChildrenIndices;
      s << e.Padding2;
      return s;
    }
  };

  struct DCElement86 {
    DCName Name;
    uint16 Index = 0;
    uint16 AttributeCount = 0;
    uint16 ChildrenCount = 0;
    DCIndex AttributeIndices;
    DCIndex ChildrenIndices;

    friend FStream& operator<<(FStream& s, DCElement86& e)
    {
      s << e.Name;
      s << e.Index;
      s << e.AttributeCount;
      s << e.ChildrenCount;
      s << e.AttributeIndices;
      s << e.ChildrenIndices;
      return s;
    }
  };

  struct DCElement {
    DCElement() = default;
    DCElement(DCElement86* el)
      : Element86(el)
    {}
    DCElement(DCElement64* el)
      : Element64(el)
    {}

    bool IsValidElement() const
    {
      return Element86 || Element64;
    }

    bool operator==(const DCElement& b) const
    {
      if (IsValidElement() != b.IsValidElement())
      {
        return false;
      }
      return GetName() == b.GetName() && GetIndex() == b.GetIndex() && GetAttributeCount() == b.GetAttributeCount() && GetChildrenCount() == b.GetChildrenCount() && GetAttributeIndices() == b.GetAttributeIndices() && GetChildrenIndices() == b.GetChildrenIndices();
    }

    DCName& GetName()
    {
      return Element64 ? Element64->Name : Element86->Name;
    }

    const DCName& GetName() const
    {
      return Element64 ? Element64->Name : Element86->Name;
    }

    uint16& GetIndex()
    {
      return Element64 ? Element64->Index : Element86->Index;
    }

    const uint16& GetIndex() const
    {
      return Element64 ? Element64->Index : Element86->Index;
    }

    uint16& GetAttributeCount()
    {
      return Element64 ? Element64->AttributeCount : Element86->AttributeCount;
    }

    const uint16& GetAttributeCount() const
    {
      return Element64 ? Element64->AttributeCount : Element86->AttributeCount;
    }

    uint16& GetChildrenCount()
    {
      return Element64 ? Element64->ChildrenCount : Element86->ChildrenCount;
    }

    const uint16& GetChildrenCount() const
    {
      return Element64 ? Element64->ChildrenCount : Element86->ChildrenCount;
    }

    DCIndex& GetAttributeIndices()
    {
      return Element64 ? Element64->AttributeIndices : Element86->AttributeIndices;
    }

    const DCIndex& GetAttributeIndices() const
    {
      return Element64 ? Element64->AttributeIndices : Element86->AttributeIndices;
    }

    DCIndex& GetChildrenIndices()
    {
      return Element64 ? Element64->ChildrenIndices : Element86->ChildrenIndices;
    }

    const DCIndex& GetChildrenIndices() const
    {
      return Element64 ? Element64->ChildrenIndices : Element86->ChildrenIndices;
    }

    DCElement64* Element64 = nullptr;
    DCElement86* Element86 = nullptr;
  };

  enum DCAttributeType : uint16 {
    AT_None,
    AT_Int,
    AT_Float,
    AT_String
  };

  struct DCAttribute64 {
    DCName Name;
    uint16 Type = 0;
    union AttributeValue {
      DCIndex IndexValue;
      struct {
        int32 IntValue;
      };
      struct {
        float FloatValue;
      };
      AttributeValue()
      {
        memset(this, 0, sizeof(AttributeValue));
      }
    } Value;
    uint32 Align = 0;

    friend FStream& operator<<(FStream& s, DCAttribute64& a)
    {
      s.SerializeBytes(&a, sizeof(DCAttribute64));
      return s;
    }
  };

  struct DCAttribute86 {
    DCName Name;
    uint16 Type = 0;
    union AttributeValue {
      DCIndex IndexValue;
      struct {
        int32 IntValue;
      };
      struct {
        float FloatValue;
      };
      AttributeValue()
      {
        memset(this, 0, sizeof(AttributeValue));
      }
    } Value;

    friend FStream& operator<<(FStream& s, DCAttribute86& a)
    {
      s.SerializeBytes(&a, sizeof(DCAttribute86));
      return s;
    }
  };

  struct DCAttribute {
    DCAttribute() = default;
    DCAttribute(DCAttribute86* attr)
      : Attribute86(attr)
    {}
    DCAttribute(DCAttribute64* attr)
      : Attribute64(attr)
    {}

    bool IsValidAttribute() const
    {
      return Attribute86 || Attribute64;
    }

    DCName& GetName()
    {
      return Attribute86 ? Attribute86->Name : Attribute64->Name;
    }

    const DCName& GetName() const
    {
      return Attribute86 ? Attribute86->Name : Attribute64->Name;
    }

    uint16& GetType()
    {
      return Attribute86 ? Attribute86->Type : Attribute64->Type;
    }

    const uint16& GetType() const
    {
      return Attribute86 ? Attribute86->Type : Attribute64->Type;
    }

    DCIndex& GetIndexValue()
    {
      return Attribute86 ? Attribute86->Value.IndexValue : Attribute64->Value.IndexValue;
    }

    const DCIndex& GetIndexValue() const
    {
      return Attribute86 ? Attribute86->Value.IndexValue : Attribute64->Value.IndexValue;
    }

    int32& GetIntValue()
    {
      return Attribute86 ? Attribute86->Value.IntValue : Attribute64->Value.IntValue;
    }

    const int32& GetIntValue() const
    {
      return Attribute86 ? Attribute86->Value.IntValue : Attribute64->Value.IntValue;
    }

    float& GetFloatValue()
    {
      return Attribute86 ? Attribute86->Value.FloatValue : Attribute64->Value.FloatValue;
    }

    const float& GetFloatValue() const
    {
      return Attribute86 ? Attribute86->Value.FloatValue : Attribute64->Value.FloatValue;
    }

    DCAttribute86* Attribute86 = nullptr;
    DCAttribute64* Attribute64 = nullptr;
  };

  template <typename T, bool TRawSerialization = false, size_t TOffset = 0, size_t TSize = 0>
  struct DCVector {
    friend FStream& operator<<(FStream& s, DCVector<T, TRawSerialization, TOffset, TSize>& vec)
    {
      uint32 size = (uint32)(TSize ? TSize : vec.Container.size());
      if (!s.IsReading())
      {
        size += TOffset;
      }
      if (!TSize)
      {
        s << size;
      }
      if (s.IsReading())
      {
        size -= TOffset;
        vec.Container.resize(size);
      }
      if (TRawSerialization)
      {
        s.SerializeBytes((void*)vec.Data(), vec.Size() * sizeof(T));
      }
      else
      {
        for (T& item : vec.Container)
        {
          s << item;
        }
      }
      return s;
    }

    void Clear()
    {
      Container.clear();
    }

    virtual size_t Size() const
    {
      return Container.size();
    }

    T& operator[](size_t idx)
    {
      return Container[idx];
    }

    void Resize(size_t newSize)
    {
      Container.resize(newSize);
    }

    const T* Data() const
    {
      return Container.data();
    }

    std::vector<T> Container;
  };

  template <typename T, bool RawSerialization = false>
  struct DCArray : DCVector<T, RawSerialization> {
    friend FStream& operator<<(FStream& s, DCArray<T, RawSerialization>& a)
    {
      s << a.ArrayCapacity;
      s << a.ArraySize;
      if (s.IsReading())
      {
        a.Container.resize(a.ArrayCapacity);
      }
      if (RawSerialization)
      {
        s.SerializeBytes((void*)a.Container.data(), a.Container.size() * sizeof(T));
      }
      else
      {
        for (T& item : a.Container)
        {
          s << item;
        }
      }
      return s;
    }

    uint32 ArrayCapacity = 0;
    uint32 ArraySize = 0;
  };

  template <uint32 InfoSize = 0>
  struct DCStringPool {
    DCVector<DCArray<wchar_t, true>> StringData;
    DCVector<DCVector<DCStringInfo, true>, false, 0, InfoSize> Info;
    DCVector<DCIndex, true, 1> Indices;

    friend FStream& operator<<(FStream& s, DCStringPool& p)
    {
      s << p.StringData;
      s << p.Info;
      s << p.Indices;
      return s;
    }
  };

  struct DCInterface {
    virtual ~DCInterface()
    {}

    virtual void Serialize(MReadStream& s) = 0;

    inline wchar_t* GetName(const DCName& name)
    {
      return GetName(*GetNameIndex(name.Index - 1));
    }
    inline wchar_t* GetName(const DCIndex& nameIndex)
    {
      return GetNamePool(nameIndex.IndexA) + nameIndex.IndexB;
    }
    inline wchar_t* GetString(const DCName& string)
    {
      return GetString(*GetStringIndex(string.Index - 1));
    }
    inline wchar_t* GetString(const DCIndex& stringIndex)
    {
      return GetStringPool(stringIndex.IndexA) + stringIndex.IndexB;
    }

    virtual DCHeader* GetHeader() = 0;

    inline DCElement GetElement(const DCIndex& idx, int32 offset = 0)
    {
      if (X86)
      {
        return GetElement86(idx, offset);
      }
      return GetElement64(idx, offset);
    }

    inline DCElement64* GetElement64(const DCIndex& idx, int32 offset = 0)
    {
      DCElement64* result = GetElementPool64(idx.IndexA) + idx.IndexB + offset;
      return result->Name.Index ? result : nullptr;
    }

    inline DCElement86* GetElement86(const DCIndex& idx, int32 offset = 0)
    {
      DCElement86* result = GetElementPool86(idx.IndexA) + idx.IndexB + offset;
      return result->Name.Index ? result : nullptr;
    }

    inline DCAttribute GetAttribute(const DCIndex& idx, int32 offset = 0)
    {
      if (X86)
      {
        return GetAttribute86(idx, offset);
      }
      return GetAttribute64(idx, offset);
    }

    inline DCAttribute64* GetAttribute64(const DCIndex& idx, int32 offset = 0)
    {
      return GetAttributePool64(idx.IndexA) + idx.IndexB + offset;
    }

    inline DCAttribute86* GetAttribute86(const DCIndex& idx, int32 offset = 0)
    {
      return GetAttributePool86(idx.IndexA) + idx.IndexB + offset;
    }

    inline DCElement GetRootElement()
    {
      DCIndex rootIndex = { 0,0 };
      return GetElement(rootIndex);
    }

    virtual DCElement64* GetRootElement64()
    {
      DCIndex rootIndex = { 0,0 };
      return GetElement64(rootIndex);
    }

    virtual DCElement86* GetRootElement86()
    {
      DCIndex rootIndex = { 0,0 };
      return GetElement86(rootIndex);
    }

    void SetIsX86(bool flag)
    {
      X86 = flag;
    }

    bool IsX86() const
    {
      return X86;
    }

    void SetDetectArchitecture(bool flag)
    {
      ArchFromHeader = flag;
    }

    void DetectArchitecture()
    {
      if (ArchFromHeader)
      {
        X86 = GetHeader()->Version <= 372000; // The value is an approximation!!! May not work correctly.
      }
    }

  protected:
    virtual wchar_t* GetStringPool(int32 index) = 0;
    virtual DCIndex* GetStringIndex(int32 index) = 0;
    virtual wchar_t* GetNamePool(int32 index) = 0;
    virtual DCIndex* GetNameIndex(int32 index) = 0;
    virtual DCElement64* GetElementPool64(int32 index) = 0;
    virtual DCAttribute64* GetAttributePool64(int32 index) = 0;
    virtual DCElement86* GetElementPool86(int32 index) = 0;
    virtual DCAttribute86* GetAttributePool86(int32 index) = 0;

  private:
    bool X86 = false;
    bool ArchFromHeader = false;
  };

  // Trivial DC implementation. Slow ~200ms. May be used in the future for editing.
  struct DataCenter : public DCInterface {
    void Serialize(MReadStream& s) override
    {
      s << Header;
      DetectArchitecture();
      s << Unk;
      if (IsX86())
      {
        s << Attributes86;
        s << Elements86;
      }
      else
      {
        s << Attributes64;
        s << Elements64;
      }
      s << Strings;
      s << Names;
    }

    DCHeader* GetHeader() override
    {
      return &Header;
    }
    
  protected:
    wchar_t* GetStringPool(int32 index) override
    {
      return Strings.StringData.Container[index].Container.data();
    }

    DCIndex* GetStringIndex(int32 index) override
    {
      return &Strings.Indices.Container[index];
    }
    
    wchar_t* GetNamePool(int32 index) override
    {
      return Names.StringData.Container[index].Container.data();
    }

    DCIndex* GetNameIndex(int32 index) override
    {
      return &Names.Indices[index];
    }

    DCElement64* GetElementPool64(int32 index) override
    {
      return Elements64[index].Container.data();
    }

    DCAttribute64* GetAttributePool64(int32 index) override
    {
      return Attributes64[index].Container.data();
    }

    DCElement86* GetElementPool86(int32 index) override
    {
      return Elements86[index].Container.data();
    }

    DCAttribute86* GetAttributePool86(int32 index) override
    {
      return Attributes86[index].Container.data();
    }

    DCHeader Header;
    DCVector<UnkStruct> Unk;
    
    DCStringPool<1024> Strings;
    DCStringPool<512> Names;

    DCVector<DCArray<DCAttribute86, true>> Attributes86;
    DCVector<DCArray<DCAttribute64, true>> Attributes64;
    DCVector<DCArray<DCElement86, true>> Elements86;
    DCVector<DCArray<DCElement64, true>> Elements64;
  };

  // Fast(<1ms) R/O DC implementation for export. 
  // Life scope of the mem stream must not be smaller than DC's one!
  struct StaticDataCenter : public DCInterface {
    void Serialize(MReadStream& s) override
    {
      s << Header;
      DetectArchitecture();
      int32 tmpSize = 0;
      int32 tmpCapacity = 0;
      // Unk
      s << tmpSize;
      AdvanceStream(s, sizeof(UnkStruct) * tmpSize);

      // Attributes
      s << tmpSize;
      if (IsX86())
      {
        Attributes86.resize(tmpSize);
        for (int32 idx = 0; idx < tmpSize; ++idx)
        {
          s << tmpCapacity;
          uint32 localTmpSize = 0;
          s << localTmpSize;
          Attributes86[idx] = (DCAttribute86*)((uint8*)s.GetAllocation() + s.GetPosition());
          AdvanceStream(s, sizeof(DCAttribute86) * tmpCapacity);
          if (!s.IsGood())
          {
            UThrow("Failed to serialize DC: Unexpected end of stream!");
          }
        }
      }
      else
      {
        Attributes64.resize(tmpSize);
        for (int32 idx = 0; idx < tmpSize; ++idx)
        {
          s << tmpCapacity;
          uint32 localTmpSize = 0;
          s << localTmpSize;
          Attributes64[idx] = (DCAttribute64*)((uint8*)s.GetAllocation() + s.GetPosition());
          AdvanceStream(s, sizeof(DCAttribute64) * tmpCapacity);
          if (!s.IsGood())
          {
            UThrow("Failed to serialize DC: Unexpected end of stream!");
          }
        }
      }


      // Elements
      s << tmpSize;
      if (IsX86())
      {
        Elements86.resize(tmpSize);
        for (int32 idx = 0; idx < tmpSize; ++idx)
        {
          s << tmpCapacity;
          uint32 localTmpSize = 0;
          s << localTmpSize;
          Elements86[idx] = (DCElement86*)((uint8*)s.GetAllocation() + s.GetPosition());
          AdvanceStream(s, sizeof(DCElement86) * tmpCapacity);
          if (!s.IsGood())
          {
            UThrow("Failed to serialize DC: Unexpected end of stream!");
          }
        }
      }
      else
      {
        Elements64.resize(tmpSize);
        for (int32 idx = 0; idx < tmpSize; ++idx)
        {
          s << tmpCapacity;
          uint32 localTmpSize = 0;
          s << localTmpSize;
          Elements64[idx] = (DCElement64*)((uint8*)s.GetAllocation() + s.GetPosition());
          AdvanceStream(s, sizeof(DCElement64) * tmpCapacity);
          if (!s.IsGood())
          {
            UThrow("Failed to serialize DC: Unexpected end of stream!");
          }
        }
      }

      // Strings
      s << tmpSize;
      Strings.resize(tmpSize);
      for (int32 idx = 0; idx < tmpSize; ++idx)
      {
        s << tmpCapacity;
        uint32 localTmpSize = 0;
        s << localTmpSize;
        Strings[idx] = (wchar_t*)((uint8*)s.GetAllocation() + s.GetPosition());
        AdvanceStream(s, sizeof(wchar_t) * tmpCapacity);
        if (!s.IsGood())
        {
          UThrow("Failed to serialize DC: Unexpected end of stream!");
        }
      }
      for (int32 idx = 0; idx < 1024; ++idx)
      {
        s << tmpSize;
        AdvanceStream(s, sizeof(DCStringInfo) * tmpSize);
        if (!s.IsGood())
        {
          UThrow("Failed to serialize DC: Unexpected end of stream!");
        }
      }
      s << tmpSize;
      StringsIndices = (DCIndex*)((uint8*)s.GetAllocation() + s.GetPosition());
      AdvanceStream(s, sizeof(DCIndex) * (tmpSize - 1));
      if (!s.IsGood())
      {
        UThrow("Failed to serialize DC: Unexpected end of stream!");
      }

      // Names
      s << tmpSize;
      Names.resize(tmpSize);
      for (int32 idx = 0; idx < tmpSize; ++idx)
      {
        s << tmpCapacity;
        uint32 localTmpSize = 0;
        s << localTmpSize;
        Names[idx] = (wchar_t*)((uint8*)s.GetAllocation() + s.GetPosition());
        AdvanceStream(s, sizeof(wchar_t) * tmpCapacity);
        if (!s.IsGood())
        {
          UThrow("Failed to serialize DC: Unexpected end of stream!");
        }
      }
      for (int32 idx = 0; idx < 512; ++idx)
      {
        s << tmpSize;
        AdvanceStream(s, sizeof(DCStringInfo) * tmpSize);
        if (!s.IsGood())
        {
          UThrow("Failed to serialize DC: Unexpected end of stream!");
        }
      }
      s << tmpSize;
      NamesIndices = (DCIndex*)((uint8*)s.GetAllocation() + s.GetPosition());
      AdvanceStream(s, sizeof(DCIndex) * (tmpSize - 1));
      if (!s.IsGood())
      {
        UThrow("Failed to serialize DC: Unexpected end of stream!");
      }
    }

    DCHeader* GetHeader() override
    {
      return &Header;
    }

  protected:
    wchar_t* GetStringPool(int32 index) override
    {
      return Strings[index];
    }

    DCIndex* GetStringIndex(int32 index) override
    {
      return &StringsIndices[index];
    }

    wchar_t* GetNamePool(int32 index) override
    {
      return Names[index];
    }

    DCIndex* GetNameIndex(int32 index) override
    {
      return &NamesIndices[index];
    }

    DCElement64* GetElementPool64(int32 index) override
    {
      return Elements64[index];
    }

    DCAttribute64* GetAttributePool64(int32 index) override
    {
      return Attributes64[index];
    }

    DCElement86* GetElementPool86(int32 index) override
    {
      return Elements86[index];
    }

    DCAttribute86* GetAttributePool86(int32 index) override
    {
      return Attributes86[index];
    }

    void AdvanceStream(FStream& s, size_t size)
    {
      s.SetPosition(s.GetPosition() + size);
    }

  protected:
    DCHeader Header;

    std::vector<wchar_t*> Names;
    DCIndex* NamesIndices = nullptr;
    std::vector<wchar_t*> Strings;
    DCIndex* StringsIndices = nullptr;

    std::vector<DCAttribute86*> Attributes86;
    std::vector<DCAttribute64*> Attributes64;
    std::vector<DCElement86*> Elements86;
    std::vector<DCElement64*> Elements64;
  };

  // Abstract DC exporter
  struct DCExporter {
    DCExporter(DCInterface* storage)
      : Storage(storage)
    {}

    virtual ~DCExporter()
    {}

    void ExportElement(const DCElement& element, const std::filesystem::path& dst)
    {
      if (element.Element64)
      {
        ExportElement(element.Element64, dst);
      }
      else
      {
        ExportElement(element.Element86, dst);
      }
    }
    virtual void ExportElement(DCElement86* element, const std::filesystem::path& dst) = 0;
    virtual void ExportElement(DCElement64* element, const std::filesystem::path& dst) = 0;

  protected:
    DCInterface* Storage = nullptr;
  };

  // Export DC as XML
  struct DCXmlExporter : DCExporter {
    using DCExporter::DCExporter;

    void ExportElement(DCElement86* element, const std::filesystem::path& dst) override
    {
      if (!element || !Storage)
      {
        return;
      }
      pugi::xml_document doc;
      CreateXmlNode(element, doc);
      std::filesystem::path p(dst);
      doc.save_file(p.replace_extension("xml").wstring().c_str());
    }

    void ExportElement(DCElement64* element, const std::filesystem::path& dst) override
    {
      if (!element || !Storage)
      {
        return;
      }
      pugi::xml_document doc;
      CreateXmlNode(element, doc);
      std::filesystem::path p(dst);
      doc.save_file(p.replace_extension("xml").wstring().c_str());
    }

  protected:
    void CreateXmlNode(DCElement86* element, pugi::xml_node& parent)
    {
      if (element && element->Name.Index)
      {
        pugi::xml_node node = parent.append_child(W2A(std::wstring(Storage->GetName(element->Name))).c_str());
        for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
        {
          CreateXmlNode(Storage->GetElement86(element->ChildrenIndices, idx), node);
        }
        CreateXmlAttributes(element, node);
      }
    }

    void CreateXmlNode(DCElement64* element, pugi::xml_node& parent)
    {
      if (element && element->Name.Index)
      {
        pugi::xml_node node = parent.append_child(W2A(std::wstring(Storage->GetName(element->Name))).c_str());
        for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
        {
          CreateXmlNode(Storage->GetElement64(element->ChildrenIndices, idx), node);
        }
        CreateXmlAttributes(element, node);
      }
    }

    void CreateXmlAttributes(DCElement86* element, pugi::xml_node& node)
    {
      if (element && element->Name.Index)
      {
        for (int32 idx = 0; idx < element->AttributeCount; ++idx)
        {
          DCAttribute86* attr = Storage->GetAttribute86(element->AttributeIndices, idx);
          if (!attr->Name.Index)
          {
            continue;
          }
          switch (attr->Type & 3)
          {
          case DCAttributeType::AT_Int:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            xattr.set_value(attr->Value.IntValue);
            break;
          }
          case DCAttributeType::AT_Float:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            xattr.set_value(attr->Value.FloatValue);
            break;
          }
          case DCAttributeType::AT_String:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            std::string tmp = W2A(Storage->GetString(attr->Value.IndexValue));
            xattr.set_value(tmp.c_str());
            break;
          }
          }
        }
      }
    }

    void CreateXmlAttributes(DCElement64* element, pugi::xml_node& node)
    {
      if (element && element->Name.Index)
      {
        for (int32 idx = 0; idx < element->AttributeCount; ++idx)
        {
          DCAttribute64* attr = Storage->GetAttribute64(element->AttributeIndices, idx);
          if (!attr->Name.Index)
          {
            continue;
          }
          switch (attr->Type & 3)
          {
          case DCAttributeType::AT_Int:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            xattr.set_value(attr->Value.IntValue);
            break;
          }
          case DCAttributeType::AT_Float:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            xattr.set_value(attr->Value.FloatValue);
            break;
          }
          case DCAttributeType::AT_String:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            std::string tmp = W2A(Storage->GetString(attr->Value.IndexValue));
            xattr.set_value(tmp.c_str());
            break;
          }
          }
        }
      }
    }
  };

  // Export DC as JSON
  struct DCJsonExporter : DCExporter {
    using DCExporter::DCExporter;

    void ExportElement(DCElement86* element, const std::filesystem::path& dst) override;
    void ExportElement(DCElement64* element, const std::filesystem::path& dst) override;

  protected:
    void CreateJsonNode(DCElement86* element, void* writer);
    void CreateJsonNode(DCElement64* element, void* writer);
  };

  inline bool operator<(const DCName& a, const DCName& b)
  {
    return a.Index < b.Index;
  }
}

namespace std
{
  template <>
  struct hash<S1Data::DCName>
  {
    std::size_t operator()(const S1Data::DCName& a) const
    {
      return a.Index;
    }
  };
}
