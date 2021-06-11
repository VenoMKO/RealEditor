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
  };

  struct DCIndex {
    uint16 IndexA = 0xFFFF;
    uint16 IndexB = 0xFFFF;
    friend FStream& operator<<(FStream& s, DCIndex& i)
    {
      return s << i.IndexA << i.IndexB;
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

  struct DCElement {
    DCName Name;
    uint16 Index = 0;
    uint16 AttributeCount = 0;
    uint16 ChildrenCount = 0;
    DCIndex AttributeIndices;
    uint32 Padding1 = 0;
    DCIndex ChildrenIndices;
    uint32 Padding2 = 0;

    friend FStream& operator<<(FStream& s, DCElement& e)
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

  struct DCAttribute {
    enum AttributeType : uint16 {
      AT_None,
      AT_Int,
      AT_Float,
      AT_String
    };

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

    friend FStream& operator<<(FStream& s, DCAttribute& a)
    {
      s.SerializeBytes(&a, sizeof(DCAttribute));
      return s;
    }
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
    inline DCElement* GetElement(const DCIndex& idx, int32 offset = 0)
    {
      DCElement* result = GetElementPool(idx.IndexA) + idx.IndexB + offset;
      return result->Name.Index ? result : nullptr;
    }
    inline DCAttribute* GetAttribute(const DCIndex& idx, int32 offset = 0)
    {
      return GetAttributePool(idx.IndexA) + idx.IndexB + offset;
    }

    virtual DCElement* GetRootElement()
    {
      DCIndex rootIndex = { 0,0 };
      return GetElement(rootIndex);
    }

  protected:
    virtual wchar_t* GetStringPool(int32 index) = 0;
    virtual DCIndex* GetStringIndex(int32 index) = 0;
    virtual wchar_t* GetNamePool(int32 index) = 0;
    virtual DCIndex* GetNameIndex(int32 index) = 0;
    virtual DCElement* GetElementPool(int32 index) = 0;
    virtual DCAttribute* GetAttributePool(int32 index) = 0;
  };

  // Trivial DC implementation. Slow ~200ms. May be used in the future for editing.
  struct DataCenter : DCInterface {
    FStream& Serialize(FStream& s)
    {
      s << Header;
      s << Unk;
      s << Attributes;
      s << Elements;
      s << Strings;
      s << Names;
      return s;
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

    DCElement* GetElementPool(int32 index) override
    {
      return Elements[index].Container.data();
    }

    DCAttribute* GetAttributePool(int32 index) override
    {
      return Attributes[index].Container.data();
    }

    DCHeader Header;
    DCVector<UnkStruct> Unk;
    DCVector<DCArray<DCAttribute, true>> Attributes;
    DCVector<DCArray<DCElement, true>> Elements;
    DCStringPool<1024> Strings;
    DCStringPool<512> Names;
  };

  // Fast(0ms) RO DC implementation for export. 
  // The stream's live scope must not be smaller than DCs scope!
  struct StaticDataCenter : DCInterface {
    void Serialize(MReadStream& s)
    {
      s << Header;
      int32 tmpSize = 0;
      int32 tmpCapacity = 0;
      // Unk
      s << tmpSize;
      AdvanceStream(s, sizeof(UnkStruct) * tmpSize);

      // Attributes
      s << tmpSize;
      Attributes.resize(tmpSize);
      for (int32 idx = 0; idx < tmpSize; ++idx)
      {
        s << tmpCapacity;
        uint32 localTmpSize = 0;
        s << localTmpSize;
        Attributes[idx] = (DCAttribute*)((uint8*)s.GetAllocation() + s.GetPosition());
        AdvanceStream(s, sizeof(DCAttribute) * tmpCapacity);
      }

      // Elements
      s << tmpSize;
      Elements.resize(tmpSize);
      for (int32 idx = 0; idx < tmpSize; ++idx)
      {
        s << tmpCapacity;
        uint32 localTmpSize = 0;
        s << localTmpSize;
        Elements[idx] = (DCElement*)((uint8*)s.GetAllocation() + s.GetPosition());
        AdvanceStream(s, sizeof(DCElement) * tmpCapacity);
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
      }
      for (int32 idx = 0; idx < 1024; ++idx)
      {
        s << tmpSize;
        AdvanceStream(s, sizeof(DCStringInfo) * tmpSize);
      }
      s << tmpSize;
      StringsIndices = (DCIndex*)((uint8*)s.GetAllocation() + s.GetPosition());
      AdvanceStream(s, sizeof(DCIndex) * (tmpSize - 1));

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
      }
      for (int32 idx = 0; idx < 512; ++idx)
      {
        s << tmpSize;
        AdvanceStream(s, sizeof(DCStringInfo) * tmpSize);
      }
      s << tmpSize;
      NamesIndices = (DCIndex*)((uint8*)s.GetAllocation() + s.GetPosition());
      AdvanceStream(s, sizeof(DCIndex) * (tmpSize - 1));
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

    DCElement* GetElementPool(int32 index) override
    {
      return Elements[index];
    }

    DCAttribute* GetAttributePool(int32 index) override
    {
      return Attributes[index];
    }

    void AdvanceStream(MReadStream& s, size_t size)
    {
      s.SetPosition(s.GetPosition() + size);
    }

  protected:
    DCHeader Header;
    std::vector<DCAttribute*> Attributes;
    std::vector<DCElement*> Elements;
    std::vector<wchar_t*> Names;
    DCIndex* NamesIndices = nullptr;
    std::vector<wchar_t*> Strings;
    DCIndex* StringsIndices = nullptr;
  };

  // Abstract DC exporter
  struct DCExporter {
    DCExporter(DCInterface* storage)
      : Storage(storage)
    {}

    virtual ~DCExporter()
    {}

    virtual void ExportElement(DCElement* element, const std::filesystem::path& dst) = 0;

  protected:
    DCInterface* Storage = nullptr;
  };

  // Export DC as XML
  struct DCXmlExporter : DCExporter {
    using DCExporter::DCExporter;

    void ExportElement(DCElement* element, const std::filesystem::path& dst) override
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
    void CreateXmlNode(DCElement* element, pugi::xml_node& parent)
    {
      if (element && element->Name.Index)
      {
        pugi::xml_node node = parent.append_child(W2A(std::wstring(Storage->GetName(element->Name))).c_str());
        for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
        {
          CreateXmlNode(Storage->GetElement(element->ChildrenIndices, idx), node);
        }
        CreateXmlAttributes(element, node);
      }
    }

    void CreateXmlAttributes(DCElement* element, pugi::xml_node& node)
    {
      if (element && element->Name.Index)
      {
        for (int32 idx = 0; idx < element->AttributeCount; ++idx)
        {
          DCAttribute* attr = Storage->GetAttribute(element->AttributeIndices, idx);
          if (!attr->Name.Index)
          {
            continue;
          }
          switch (attr->Type & 3)
          {
          case DCAttribute::AT_Int:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            xattr.set_value(attr->Value.IntValue);
            break;
          }
          case DCAttribute::AT_Float:
          {
            pugi::xml_attribute xattr = node.append_attribute(W2A(Storage->GetName(attr->Name)).c_str());
            xattr.set_value(attr->Value.FloatValue);
            break;
          }
          case DCAttribute::AT_String:
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

    void ExportElement(DCElement* element, const std::filesystem::path& dst) override;

  protected:
    void CreateJsonNode(DCElement* element, void* writer);
  
  };

  inline bool operator<(const DCName& a, const DCName& b)
  {
    return a.Index < b.Index;
  }

  inline bool operator==(const DCName& a, const DCName& b)
  {
    return a.Index == b.Index;
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
