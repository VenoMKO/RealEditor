#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

#include "DC.h"

void S1Data::DCJsonExporter::ExportElement(DCElement86* element, const std::filesystem::path& dst)
{
  if (!element || !Storage)
  {
    return;
  }

  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter writer(sb);
  writer.SetIndent(' ', 2);

  CreateJsonNode(element, (void*)&writer);

  std::filesystem::path p(dst);
  FWriteStream s(p.replace_extension("json").wstring());
  s.SerializeBytes((void*)sb.GetString(), sb.GetSize());
}

void S1Data::DCJsonExporter::ExportElement(DCElement64* element, const std::filesystem::path& dst)
{
  if (!element || !Storage)
  {
    return;
  }

  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter writer(sb);
  writer.SetIndent(' ', 2);

  CreateJsonNode(element, (void*)&writer);

  std::filesystem::path p(dst);
  FWriteStream s(p.replace_extension("json").wstring());
  s.SerializeBytes((void*)sb.GetString(), sb.GetSize());
}

void S1Data::DCJsonExporter::CreateJsonNode(DCElement86* element, void* writerUntyped)
{
  // rapidjson conflicts with wxWidgets new under DEBUG. Use void* to fix it.
  auto* writer = (rapidjson::PrettyWriter<rapidjson::StringBuffer>*)writerUntyped;
  writer->StartObject();
  if (element->AttributeCount)
  {
    for (int32 idx = 0; idx < element->AttributeCount; ++idx)
    {
      DCAttribute86* attr = Storage->GetAttribute86(element->AttributeIndices, idx);
      if (!attr->Name.Index || !attr->Type)
      {
        continue;
      }
      writer->Key(W2A(Storage->GetName(attr->Name)).c_str());
      switch (attr->Type & 3)
      {
      case DCAttributeType::AT_Int:
      {
        writer->Int(attr->Value.IntValue);
        break;
      }
      case DCAttributeType::AT_Float:
      {
        writer->Double(attr->Value.FloatValue);
        break;
      }
      case DCAttributeType::AT_String:
      {
        writer->String(W2A(Storage->GetString(attr->Value.IndexValue)).c_str());
        break;
      }
      }
    }
  }
  if (element->ChildrenCount)
  {
    bool foundName = false;
    for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
    {
      if (DCElement86* child = Storage->GetElement86(element->ChildrenIndices, idx))
      {
        writer->String(W2A(Storage->GetName(child->Name)).c_str());
        foundName = true;
        break;
      }
    }
    if (!foundName)
    {
      writer->String(W2A(Storage->GetName(element->Name)).c_str());
    }
    writer->StartArray();
    for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
    {
      CreateJsonNode(Storage->GetElement86(element->ChildrenIndices, idx), writer);
    }
    writer->EndArray();
  }
  writer->EndObject();
}

void S1Data::DCJsonExporter::CreateJsonNode(DCElement64* element, void* writerUntyped)
{
  // rapidjson conflicts with wxWidgets new under DEBUG. Use void* to fix it.
  auto* writer = (rapidjson::PrettyWriter<rapidjson::StringBuffer>*)writerUntyped;
  writer->StartObject();
  if (element->AttributeCount)
  {
    for (int32 idx = 0; idx < element->AttributeCount; ++idx)
    {
      DCAttribute64* attr = Storage->GetAttribute64(element->AttributeIndices, idx);
      if (!attr->Name.Index || !attr->Type)
      {
        continue;
      }
      writer->Key(W2A(Storage->GetName(attr->Name)).c_str());
      switch (attr->Type & 3)
      {
      case DCAttributeType::AT_Int:
      {
        writer->Int(attr->Value.IntValue);
        break;
      }
      case DCAttributeType::AT_Float:
      {
        writer->Double(attr->Value.FloatValue);
        break;
      }
      case DCAttributeType::AT_String:
      {
        writer->String(W2A(Storage->GetString(attr->Value.IndexValue)).c_str());
        break;
      }
      }
    }
  }
  if (element->ChildrenCount)
  {
    bool foundName = false;
    for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
    {
      if (DCElement64* child = Storage->GetElement64(element->ChildrenIndices, idx))
      {
        writer->String(W2A(Storage->GetName(child->Name)).c_str());
        foundName = true;
        break;
      }
    }
    if (!foundName)
    {
      writer->String(W2A(Storage->GetName(element->Name)).c_str());
    }
    writer->StartArray();
    for (int32 idx = 0; idx < element->ChildrenCount; ++idx)
    {
      CreateJsonNode(Storage->GetElement64(element->ChildrenIndices, idx), writer);
    }
    writer->EndArray();
  }
  writer->EndObject();
}

