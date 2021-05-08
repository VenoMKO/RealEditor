#include "MaterialMapperDialog.h"
#include <Tera/UObject.h>

struct MaterialMapperItem {
  wxString FbxName;
  UObject* Material = nullptr;
};

class MaterialMapperModel : public wxDataViewListModel {
public:
  MaterialMapperModel(const std::vector<MaterialMapperItem>& map, const std::vector<UObject*>& objectMaterials)
  {
    Materials = map;
    ObjectMaterials = objectMaterials;
    if (ObjectMaterials.front())
    {
      ObjectMaterials.insert(ObjectMaterials.begin(), nullptr);
    }
  }

  unsigned int GetColumnCount() const override
  {
    return 2;
  }

  bool HasValue(const wxDataViewItem& item, unsigned col) const override
  {
    return Materials.size();
  }

  void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override
  {
    MaterialMapperItem* mat = (MaterialMapperItem*)item.GetID(); 
    if (!col)
    {
      wxString name = mat->FbxName;
      variant = name;
    }
    else
    {
      long value = 0;
      auto it = std::find(ObjectMaterials.begin(), ObjectMaterials.end(), mat->Material);
      if (it != ObjectMaterials.end())
      {
        value = std::distance(ObjectMaterials.begin(), it);
      }
      variant = value;
    }
  }

  bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col) override
  {
    long value = variant.GetInteger();
    Materials[GetRow(item)].Material = ObjectMaterials[value];
    return true;
  }

  wxString GetColumnType(unsigned int col) const override
  {
    return col ? "long" : "string";
  }

  bool IsEnabled(const wxDataViewItem& item, unsigned int col) const override
  {
    return true;
  }

  wxDataViewItem GetParent(const wxDataViewItem& item) const override
  {
    return wxDataViewItem(nullptr);
  }

  bool IsContainer(const wxDataViewItem& item) const override
  {
    return item.GetID() ? false : true;
  }

  unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& array) const override
  {
    if (!item.GetID())
    {
      for (const auto & material : Materials)
      {
        array.Add(wxDataViewItem((void*)&material));
      }
      return Materials.size();
    }
    return 0;
  }

  unsigned int GetCount() const override
  {
    return Materials.size();
  }

  unsigned int GetRow(const wxDataViewItem& item) const override
  {
    for (int32 idx = 0; idx < Materials.size(); ++idx)
    {
      if (&Materials[idx] == (MaterialMapperItem*)item.GetID())
      {
        return idx;
      }
    }
    return 0;
  }

  void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override
  {
    if (!col)
    {
      wxString name = Materials[row].FbxName;
      variant = name;
    }
    else
    {
      long value = 0;
      auto it = std::find(ObjectMaterials.begin(), ObjectMaterials.end(), Materials[row].Material);
      if (it != ObjectMaterials.end())
      {
        value = std::distance(ObjectMaterials.begin(), it);
      }
      variant = value;
    }
  }

  bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override
  {
    long value = variant.GetInteger();
    Materials[row].Material = ObjectMaterials[value];
    return true;
  }

  std::vector<MaterialMapperItem> GetMaterials() const
  {
    return Materials;
  }

protected:
  std::vector<MaterialMapperItem> Materials;
  std::vector<UObject*> ObjectMaterials;
};

bool MaterialMapperDialog::AutomaticallyMapMaterials(std::vector<class FString>& fbxMaterials, const std::vector<UObject*>& objectMaterials, std::vector<std::pair<FString, UObject*>>& output)
{
  output.resize(fbxMaterials.size());
  std::vector<bool> exact(fbxMaterials.size());
  for (int32 fbxIdx = 0; fbxIdx < fbxMaterials.size(); ++fbxIdx)
  {
    int32 dist = INT_MAX;
    const wxString fbxName = fbxMaterials[fbxIdx].ToUpper().WString();
    for (auto objectMaterial : objectMaterials)
    {
      int32 cmp = fbxName.Cmp(objectMaterial->GetObjectName().ToUpper().UTF8().c_str());
      if (cmp >= 0 && dist > cmp)
      {
        exact[fbxIdx] = !cmp;
        dist = cmp;
        output[fbxIdx] = std::make_pair(fbxMaterials[fbxIdx], objectMaterial);
      }
    }
  }
  bool exactMatch = true;
  for (bool e : exact)
  {
    if (!e)
    {
      exactMatch = false;
      break;
    }
  }
  return exactMatch;
}

MaterialMapperDialog::MaterialMapperDialog(wxWindow* parent, const std::vector<std::pair<FString, UObject*>>& map, const std::vector<UObject*>& objectMaterials)
  : wxDialog(parent, wxID_ANY, wxT("Material mapping"), wxDefaultPosition, wxSize(462, 333), wxDEFAULT_DIALOG_STYLE)
{
  ObjectMaterials = objectMaterials;
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText1;
  m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("Match FBX materials with the object materials:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText1->Wrap(-1);
  bSizer1->Add(m_staticText1, 0, wxALL, 5);

  MaterialsList = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
  bSizer1->Add(MaterialsList, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer* bSizer3;
  bSizer3 = new wxBoxSizer(wxHORIZONTAL);

  wxButton* m_button5;
  m_button5 = new wxButton(this, wxID_ANY, wxT("Edit..."), wxDefaultPosition, wxDefaultSize, 0);
  m_button5->Enable(false);

  bSizer3->Add(m_button5, 0, wxALL, 5);


  bSizer3->Add(0, 0, 1, wxEXPAND, 5);

  wxButton* m_button3;
  m_button3 = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer3->Add(m_button3, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  wxButton* m_button4;
  m_button4 = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer3->Add(m_button4, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  bSizer1->Add(bSizer3, 0, wxEXPAND | wxTOP | wxBOTTOM, 15);


  this->SetSizer(bSizer1);
  this->Layout();

  this->Centre(wxBOTH);

  // Connect Events
  m_button5->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MaterialMapperDialog::OnEditClicked), nullptr, this);
  m_button3->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MaterialMapperDialog::OnOkClicked), nullptr, this);
  m_button4->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MaterialMapperDialog::OnCancelClicked), nullptr, this);

  MaterialsList->AppendTextColumn(wxT("FBX Material"), wxDATAVIEW_CELL_INERT, 210, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE);

  // Populate choices
  wxArrayString choices;
  choices.Add("None");
  for (const auto& mat : objectMaterials)
  {
    if (!mat)
    {
      continue;
    }
    choices.Add(mat->GetObjectName().WString());
  }
  wxDataViewColumn* m_dataViewListColumn1 = new wxDataViewColumn(wxT("Object Material"), new wxDataViewChoiceByIndexRenderer(choices), wxDATAVIEW_CELL_EDITABLE, 210, static_cast<wxAlignment>(wxALIGN_LEFT));
  MaterialsList->AppendColumn(m_dataViewListColumn1);
  std::vector<MaterialMapperItem> items;
  for (const auto& p : map)
  {
    items.push_back({ wxString(p.first.WString()), p.second });
  }
  wxDataViewModel* model = new MaterialMapperModel(items, objectMaterials);
  MaterialsList->AssociateModel(model);
  model->DecRef();
}

std::vector<std::pair<FString, UObject*>> MaterialMapperDialog::GetResult() const
{
  MaterialMapperModel* model = (MaterialMapperModel*)MaterialsList->GetModel();
  auto materials = model->GetMaterials();
  std::vector<std::pair<FString, UObject*>> result;
  for (const auto& item : materials)
  {
    result.emplace_back(std::make_pair(FString(item.FbxName.ToStdWstring()), item.Material));
  }
  return result;
}

void MaterialMapperDialog::OnEditClicked(wxCommandEvent&)
{
  // TODO: Modify model materials
}

void MaterialMapperDialog::OnOkClicked(wxCommandEvent&)
{
  EndModal(wxID_OK);
}

void MaterialMapperDialog::OnCancelClicked(wxCommandEvent&)
{
  EndModal(wxID_CANCEL);
}
