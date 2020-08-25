#include "ObjectProperties.h"

#include <Tera/FObjectResource.h>
#include <Tera/FPackage.h>
#include <Tera/UClass.h>

inline wxString GetPropertyName(FPropertyValue* v, int32 idx = -1)
{
  if (idx != -1)
  {
    return wxString::Format("[%d]", idx);
  }
  if (v->Property->StaticArrayNext || v->Property->ArrayDim > 1)
  {
    return wxString::Format("[%d]", v->Property->ArrayIndex);
  }
  if (v->Type == FPropertyValue::VID::Struct)
  {
    return v->Property->Name.String().String();
  }
  return v->Field ? v->Field->GetObjectName().WString() : v->Property->Name.String().WString();
}

inline wxString GetPropertyId(FPropertyValue* v)
{
  return wxString::Format("%16llx", (uint64)std::addressof(*v));
}

inline wxArrayString GetEnumLabels(FPropertyValue* v)
{
  wxArrayString labels;
  const int32 maxEnum = v->Enum->NumEnums() - 1;
  for (int32 i = 0; i < maxEnum; ++i)
  {
    labels.Add(v->Enum->GetEnum(i).String().String());
  }
  return labels;
}

inline wxArrayInt GetEnumValues(FPropertyValue* v)
{
  wxArrayInt values;
  const int32 maxEnum = v->Enum->NumEnums() - 1;
  for (int32 i = 0; i < maxEnum; ++i)
  {
    values.Add(i);
  }
  return values;
}

inline wxString GetObjectNameForIndex(FPropertyValue* value)
{
  if (PACKAGE_INDEX idx = value->GetObjectIndex())
  {
    return value->Property->Owner->GetPackage()->GetResourceObject(idx)->GetFullObjectName().WString();
  }
  return wxT("NULL");
}

void CreateProperty(wxPropertyGridManager* mgr, wxPropertyCategory* cat, const std::vector<FPropertyTag*>& properties)
{
  std::vector<wxPropertyCategory*> cats = { cat };
  int32 remainingArrayDim = properties.empty() ? 0 : properties.front()->ArrayDim;
  wxPropertyCategory* category = nullptr;
  int32 prevArrayIndex = 0;
  for (FPropertyTag* tag : properties)
  {
    if (--remainingArrayDim <= 0 || prevArrayIndex > tag->ArrayIndex)
    {
      category = cats.back();
      cats.pop_back();

      remainingArrayDim = tag->ArrayDim;

      if (remainingArrayDim > 1)
      {
        wxPropertyCategory* newCategory = new wxPropertyCategory(tag->Name.String().String(), tag->Name.String().String());
        newCategory->Enable(category->IsEnabled());
        newCategory->SetExpanded(false);
        mgr->AppendIn(category, newCategory);
        cats.push_back(category);
        category = newCategory;
      }
      else
      {
        cats.push_back(category);
      }
    }
    CreateProperty(mgr, category, tag->Value);
    prevArrayIndex = tag->ArrayIndex;
  }
}

void CreateProperty(wxPropertyGridManager* mgr, wxPropertyCategory* cat, FPropertyValue* value, int32 idx)
{
  if (value->Type == FPropertyValue::VID::Int)
  {
    auto pgp = new AIntProperty(value, idx);
    pgp->Enable(cat->IsEnabled());
    mgr->AppendIn(cat, pgp);
  }
  else if (value->Type == FPropertyValue::VID::Float)
  {
    auto pgp = new AFloatProperty(value, idx);
    pgp->Enable(cat->IsEnabled());
    mgr->AppendIn(cat, pgp);
  }
  else if (value->Type == FPropertyValue::VID::Bool)
  {
    auto pgp = new ABoolProperty(value, idx);
    pgp->Enable(cat->IsEnabled());
    mgr->AppendIn(cat, pgp);
  }
  else if (value->Type == FPropertyValue::VID::Byte)
  {
    if (value->Enum)
    {
      auto pgp = new AEnumProperty(value, idx);
      pgp->Enable(cat->IsEnabled());
      mgr->AppendIn(cat, pgp);
    }
    else
    {
      auto pgp = new AByteProperty(value, idx);
      pgp->Enable(cat->IsEnabled());
      mgr->AppendIn(cat, pgp);
    }
  }
  else if (value->Type == FPropertyValue::VID::String)
  {
    auto pgp = new AStringProperty(value, idx);
    pgp->Enable(cat->IsEnabled());
    mgr->AppendIn(cat, pgp);
  }
  else if (value->Type == FPropertyValue::VID::Name)
  {
    auto pgp = new ANameProperty(value, idx);
    pgp->Enable(cat->IsEnabled());
    mgr->AppendIn(cat, pgp);
  }
  else if (value->Type == FPropertyValue::VID::Object)
  {
    auto pgp = new AObjectProperty(value, idx);
    pgp->Enable(cat->IsEnabled());
    mgr->AppendIn(cat, pgp);
  }
  else if (value->Type == FPropertyValue::VID::Property)
  {
    CreateProperty(mgr, cat, { value->GetPropertyTagPtr() });
  }
  else if (value->Type == FPropertyValue::VID::Field)
  {
    std::vector<FPropertyValue*> arr = value->GetArray();
    if (arr.size() == 1)
    {
      for (FPropertyValue* v : arr)
      {
        CreateProperty(mgr, cat, v);
      }
    }
    else
    {
      for (int32 aidx = 0; aidx < arr.size(); ++aidx)
      {
        CreateProperty(mgr, cat, arr[aidx], aidx);
      }
    }
  }
  else if (value->Type == FPropertyValue::VID::Array)
  {
    std::vector<FPropertyValue*> arr = value->GetArray();

    if (arr.size() && arr[0]->Type == FPropertyValue::VID::Byte && !arr[0]->Enum)
    {
      // Special case for byte arrays
      wxStringProperty* pgp = new wxStringProperty(value->Property->Name.String().WString(), GetPropertyId(value), wxString::Format("%lluKb", arr.size() / 1024));
      pgp->Enable(cat->IsEnabled());
      if (value->Field && value->Field->GetToolTip().Size())
      {
        pgp->SetHelpString(value->Field->GetToolTip().WString());
      }
      mgr->AppendIn(cat, pgp);
    }
    else
    {
      wxPropertyCategory* ncat = new wxPropertyCategory(idx >= 0 ? wxString::Format("%d", idx) : value->Property->Name.String().WString(), GetPropertyId(value));
      ncat->Enable(cat->IsEnabled());
      ncat->SetExpanded(false);
      if (idx >= 0)
      {
        ncat->SetHelpString(value->Property->Name.String().WString() + wxString::Format("[%d]", idx));
      }
      mgr->AppendIn(cat, ncat);
      if (value->Field && value->Field->GetToolTip().Size())
      {
        ncat->SetHelpString(value->Field->GetToolTip().WString());
      }
      else if (value->Struct && value->Struct->GetToolTip().Size())
      {
        ncat->SetHelpString(value->Struct->GetToolTip().WString());
      }

      for (int32 aidx = 0; aidx < arr.size(); ++aidx)
      {
        CreateProperty(mgr, ncat, arr[aidx], aidx);
      }
    }
  }
  else if (value->Type == FPropertyValue::VID::Struct)
  {
    wxPropertyCategory* ncat = new wxPropertyCategory(idx >= 0 ? wxString::Format("%d", idx) : value->Property->Name.String().WString(), GetPropertyId(value));
    ncat->Enable(cat->IsEnabled());
    mgr->AppendIn(cat, ncat);
    bool hasDesc = false;
    if (value->Struct)
    {
      ncat->SetValue(value->Struct->GetObjectName().String());
      if (value->Struct->GetToolTip().Size())
      {
        hasDesc = true;
        ncat->SetHelpString(value->Struct->GetToolTip().WString());
      }
    }
    if (value->Field && value->Field->GetToolTip().Size())
    {
      hasDesc = true;
      ncat->SetHelpString(value->Field->GetToolTip().WString());
    }
    if (idx >= 0 && !hasDesc)
    {
      ncat->SetHelpString(value->Property->Name.String().WString() + wxString::Format("[%d]", idx));
    }
    std::vector<FPropertyValue*> arr = value->GetArray();

    if (arr.size())
    {
      if (arr.front()->Type == FPropertyValue::VID::Property)
      {
        std::vector<FPropertyTag*> props;
        for (FPropertyValue* v : arr)
        {
          props.push_back(v->GetPropertyTagPtr());
        }
        CreateProperty(mgr, ncat, props);
      }
      else
      {
        for (FPropertyValue* v : arr)
        {
          CreateProperty(mgr, ncat, v);
        }
      }
    }
  }
}



AIntProperty::AIntProperty(FPropertyValue* value, int32 idx)
  : wxIntProperty(GetPropertyName(value, idx), GetPropertyId(value), value->GetInt())
  , Value(value)
{
  if (value->Field && value->Field->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
  else if (value->Struct && value->Struct->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
}

AFloatProperty::AFloatProperty(FPropertyValue* value, int32 idx)
  : wxFloatProperty(GetPropertyName(value, idx), GetPropertyId(value), value->GetFloat())
  , Value(value)
{
  if (value->Field && value->Field->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
  else if (value->Struct && value->Struct->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
}

ABoolProperty::ABoolProperty(FPropertyValue* value, int32 idx)
  : wxBoolProperty(GetPropertyName(value, idx), GetPropertyId(value), value->GetBool())
  , Value(value)
{
  if (value->Field && value->Field->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
  else if (value->Struct && value->Struct->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
}

AByteProperty::AByteProperty(FPropertyValue* value, int32 idx)
  : wxIntProperty(GetPropertyName(value, idx), GetPropertyId(value), value->GetByte())
  , Value(value)
{
  if (value->Field && value->Field->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
  else if (value->Struct && value->Struct->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
}

AStringProperty::AStringProperty(FPropertyValue* value, int32 idx)
  : wxStringProperty(GetPropertyName(value, idx), GetPropertyId(value), value->GetString().WString())
  , Value(value)
{
  if (value->Field && value->Field->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
  else if (value->Struct && value->Struct->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
}

ANameProperty::ANameProperty(FPropertyValue* value, int32 idx)
  : wxStringProperty(GetPropertyName(value, idx), GetPropertyId(value), value->GetName().String().WString())
  , Value(value)
{
  if (value->Field && value->Field->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
  else if (value->Struct && value->Struct->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
}

AEnumProperty::AEnumProperty(FPropertyValue* value, int32 idx)
  : wxEnumProperty(GetPropertyName(value, idx), GetPropertyId(value), GetEnumLabels(value), GetEnumValues(value), value->GetByte())
  , Value(value)
{
  if (value->Field && value->Field->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
  else if (value->Struct && value->Struct->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
}

AObjectProperty::AObjectProperty(FPropertyValue* value, int32 idx)
  : wxStringProperty(GetPropertyName(value, idx), GetPropertyId(value), GetObjectNameForIndex(value))
  , Value(value)
{
  if (value->Field && value->Field->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
  else if (value->Struct && value->Struct->GetToolTip().Size())
  {
    SetHelpString(value->Field->GetToolTip().WString());
  }
}