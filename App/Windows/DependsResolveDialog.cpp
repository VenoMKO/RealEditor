#include "DependsResolveDialog.h"
#include "ObjectPicker.h"

#include <Tera/UObject.h>

struct DependsItem {
  UObject* Source = nullptr;
  long Action = 0;
  UObject* Destination = nullptr;
};

class DepResolverModel : public wxDataViewListModel {
public:
  DepResolverModel(const std::map<UObject*, UObject*>& objects)
  {
    for (auto& p : objects)
    {
      Objects.push_back({ p.first, p.second ? 1 : 0, p.second });
    }
    std::sort(Objects.begin(), Objects.end(), [](const auto& a, const auto& b) {
      return a.Source->GetObjectName() < b.Source->GetObjectName();
    });
  }

  unsigned int GetColumnCount() const override
  {
    return 3;
  }

  bool HasValue(const wxDataViewItem& item, unsigned col) const override
  {
    return true;
  }

  bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col) override
  {
    long newVal = variant.GetInteger();
    if (newVal != Objects[GetRow(item)].Action)
    {
      Objects[GetRow(item)].Action = newVal;
      Objects[GetRow(item)].Destination = nullptr;
    }
    return true;
  }

  wxString GetColumnType(unsigned int col) const override
  {
    if (col == 1)
    {
      return "long";
    }
    return "string";
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
      for (const auto & obj : Objects)
      {
        array.Add(wxDataViewItem((void*)&obj));
      }
      return Objects.size();
    }
    return 0;
  }

  unsigned int GetCount() const override
  {
    return Objects.size();
  }

  unsigned int GetRow(const wxDataViewItem& item) const override
  {
    for (int32 idx = 0; idx < Objects.size(); ++idx)
    {
      if (&Objects[idx] == (DependsItem*)item.GetID())
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
      wxString name = Objects[row].Source->GetObjectNameString().WString();
      variant = name;
    }
    else if (col == 1)
    {
      variant = Objects[row].Action;
    }
    else
    {
      wxString name = wxT("None");
      if (Objects[row].Destination)
      {
        if (Objects[row].Destination->GetClassName() == NAME_Package)
        {
          name = Objects[row].Destination->GetObjectPath().WString();
        }
        else
        {
          name = Objects[row].Destination->GetObjectNameString().WString();
        }
      }
      variant = name;
    }
  }

  void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override
  {
    DependsItem* dep = (DependsItem*)item.GetID();
    if (!col)
    {
      wxString name = dep->Source->GetObjectNameString().WString();
      variant = name;
    }
    else if (col == 1)
    {
      variant = dep->Action;
    }
    else
    {
      wxString name = wxT("None");
      if (dep->Destination)
      {
        if (dep->Destination->GetClassName() == NAME_Package)
        {
          name = dep->Destination->GetObjectPath().WString();
        }
        else
        {
          name = dep->Destination->GetObjectNameString().WString();
        }
      }
      variant = name;
    }
  }

  bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override
  {
    Objects[row].Action = variant.GetInteger();
    return true;
  }

  std::map<UObject*, UObject*> GetMap() const
  {
    std::map<UObject*, UObject*> result;
    for (const auto& item : Objects)
    {
      result[item.Source] = item.Destination;
    }
    return result;
  }

  std::vector<DependsItem>& GetData()
  {
    return Objects;
  }

protected:
  std::vector<DependsItem> Objects;
};

DependsResolveDialog::DependsResolveDialog(wxWindow* parent, const std::map<UObject*, UObject*>& objects, FPackage* destPackage)
  : WXDialog(parent, wxID_ANY, wxT("Copy Dependencies"), wxDefaultPosition, wxSize(630, 403))
  , DestinationPackage(destPackage)
{
  SetSize(FromDIP(GetSize()));
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText1;
  m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("The object you are trying to copy has some dependencies. Specify a destination folder for each of the objects below. Or press the All button to copy all dependencies to a single folder. \nAlso you can select an existing object instead of copying. To do that change the Action from Copy to Existing and press the Edit button."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText1->Wrap(FromDIP(600));
  bSizer1->Add(m_staticText1, 0, wxALL, FromDIP(5));

  DataView = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
  wxDataViewColumn* m_dataViewColumn1;
  m_dataViewColumn1 = DataView->AppendTextColumn(wxT("Object"), 0, wxDATAVIEW_CELL_INERT, FromDIP(245), static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE);

  // Populate choices
  wxArrayString choices;
  choices.Add("Copy");
  choices.Add("Assign");
  wxDataViewColumn* m_dataViewColumn3 = new wxDataViewColumn(wxT("Action"), new wxDataViewChoiceByIndexRenderer(choices), 1, -1, static_cast<wxAlignment>(wxALIGN_LEFT));
  DataView->AppendColumn(m_dataViewColumn3);
  wxDataViewColumn* m_dataViewColumn2;
  m_dataViewColumn2 = DataView->AppendTextColumn(wxT("Name"), 2, wxDATAVIEW_CELL_INERT, FromDIP(245), static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE);
  bSizer1->Add(DataView, 1, wxALL | wxEXPAND, FromDIP(5));

  wxStaticText* m_staticText2;
  m_staticText2 = new wxStaticText(this, wxID_ANY, wxT("To set a destination folder select an object in the table and press the Edit button."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText2->Wrap(-1);
  bSizer1->Add(m_staticText2, 0, wxALL, FromDIP(5));

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxHORIZONTAL);

  AllButton = new wxButton(this, wxID_ANY, wxT("All..."), wxDefaultPosition, wxDefaultSize, 0);
  bSizer2->Add(AllButton, 0, wxALL, FromDIP(5));

  EditButton = new wxButton(this, wxID_ANY, wxT("Edit..."), wxDefaultPosition, wxDefaultSize, 0);
  EditButton->Enable(false);
  bSizer2->Add(EditButton, 0, wxALL, FromDIP(5));


  bSizer2->Add(0, 0, 1, wxEXPAND, FromDIP(5));

  OkButton = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  OkButton->Enable(false);

  bSizer2->Add(OkButton, 0, wxALL, FromDIP(5));

  CancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer2->Add(CancelButton, 0, wxALL, FromDIP(5));


  bSizer1->Add(bSizer2, 0, wxEXPAND | wxTOP | wxBOTTOM, 15);


  this->SetSizer(bSizer1);
  this->Layout();

  this->Centre(wxBOTH);

  DataView->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED, wxDataViewEventHandler(DependsResolveDialog::OnDataViewValueChanged), nullptr, this);
  DataView->Connect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(DependsResolveDialog::OnSelectionChanged), nullptr, this);
  AllButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DependsResolveDialog::OnAllClicked), nullptr, this);
  EditButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DependsResolveDialog::OnEditClicked), nullptr, this);
  OkButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DependsResolveDialog::OnOkClicked), nullptr, this);
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DependsResolveDialog::OnCancelClicked), nullptr, this);

  DepResolverModel* model = new DepResolverModel(objects);
  DataView->AssociateModel(model);
  model->DecRef();

  UpdateOkButton();
}

std::map<UObject*, UObject*> DependsResolveDialog::GetResult() const
{
  return ((DepResolverModel*)DataView->GetModel())->GetMap();
}

void DependsResolveDialog::OnDataViewValueChanged(wxDataViewEvent& event)
{
  DataView->OnColumnChange(2);
  UpdateOkButton();
}

void DependsResolveDialog::OnSelectionChanged(wxDataViewEvent& event)
{
  EditButton->Enable(DataView->HasSelection());
}

void DependsResolveDialog::OnAllClicked(wxCommandEvent& event)
{
  ObjectPicker dlg(this, wxT("Select a folder..."), false, DestinationPackage->Ref(), 0, { NAME_Package });
  dlg.SetAllowRootExport(true);
  dlg.SetAllowNewPackage(true);
  if (dlg.ShowModal() != wxID_OK)
  {
    return;
  }

  UObject* dest = dlg.GetSelectedObject();
  std::vector<DependsItem>& data = ((DepResolverModel*)DataView->GetModel())->GetData();
  for (auto& item : data)
  {
    item.Action = 0;
    item.Destination = dest;
  }
  DataView->OnColumnChange(1);
  DataView->OnColumnChange(2);
  UpdateOkButton();
}

void DependsResolveDialog::OnEditClicked(wxCommandEvent& event)
{
  if (!DataView->HasSelection())
  {
    return;
  }
  wxDataViewItem selection = DataView->GetSelection();
  if (DependsItem* item = (DependsItem*)selection.GetID())
  {
    std::vector<FString> filter;
    wxString title;
    bool allowNewFolders = false;
    if (item->Action == 0)
    {
      filter.emplace_back(NAME_Package);
      title = wxT("Select a folder...");
      allowNewFolders = true;
    }
    else
    {
      filter.emplace_back(item->Source->GetClassNameString());
      title = wxT("Select a ") + item->Source->GetClassNameString().WString() + wxT(" object...");
    }
    ObjectPicker dlg(this, title, false, DestinationPackage->Ref(), DestinationPackage->GetObjectIndex(item->Destination), filter);
    dlg.SetAllowNewPackage(allowNewFolders);
    if (dlg.ShowModal() != wxID_OK)
    {
      return;
    }
    item->Destination = dlg.GetSelectedObject();
    DataView->OnColumnChange(2);
  }
  UpdateOkButton();
}

void DependsResolveDialog::OnOkClicked(wxCommandEvent& event)
{
  EndModal(wxID_OK);
}

void DependsResolveDialog::OnCancelClicked(wxCommandEvent& event)
{
  EndModal(wxID_CANCEL);
}

void DependsResolveDialog::UpdateOkButton()
{
  std::map<UObject*, UObject*> map = ((DepResolverModel*)DataView->GetModel())->GetMap();
  for (const auto& p : map)
  {
    if (!p.second)
    {
      OkButton->Enable(false);
      return;
    }
  }
  OkButton->Enable(true);
}
