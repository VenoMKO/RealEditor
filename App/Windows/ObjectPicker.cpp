#include "ObjectPicker.h"

#include <wx/statline.h>

#include "../App.h"
#include "../Misc/ObjectTreeModel.h"
#include "CompositePackagePicker.h"
#include "REDialogs.h"

#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UObject.h>
#include <Tera/UClass.h>

ObjectPicker::ObjectPicker(wxWindow* parent, const wxString& title, bool allowDifferentPackage, const wxString& packageName, PACKAGE_INDEX selection, const std::vector<FString>& allowedClasses)
  : WXDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(441, 438))
  , AllowDifferentPackage(allowDifferentPackage)
{
  SetSize(FromDIP(GetSize()));
  Filter = allowedClasses;
  SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxVERTICAL);

  ObjectTreeCtrl = new ObjectTreeDataViewCtrl(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(250, -1)), 0);
  bSizer2->Add(ObjectTreeCtrl, 1, wxRIGHT | wxEXPAND, 1);

  wxBoxSizer* bSizer4;
  bSizer4 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel6;
  m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(-1, 60)), wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);

  bSizer11->SetMinSize(FromDIP(wxSize(-1, 60)));
  PackageButton = new wxButton(m_panel6, wxID_ANY, wxT("Package..."), wxDefaultPosition, wxDefaultSize, 0);
  PackageButton->Enable(AllowDifferentPackage);

  bSizer11->Add(PackageButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


  bSizer11->Add(0, 0, 1, wxEXPAND, FromDIP(5));

  OkButton = new wxButton(m_panel6, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  OkButton->Enable(false);

  bSizer11->Add(OkButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

  CancelButton = new wxButton(m_panel6, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer11->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


  m_panel6->SetSizer(bSizer11);
  m_panel6->Layout();
  bSizer4->Add(m_panel6, 1, wxALL, 0);


  bSizer2->Add(bSizer4, 0, wxEXPAND, FromDIP(5));


  SetSizer(bSizer2);
  Layout();

  Centre(wxBOTH);

  // Connect Events
  ObjectTreeCtrl->Connect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(ObjectPicker::OnObjectSelected), NULL, this);
  PackageButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnPackageClicked), NULL, this);
  OkButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnOkClicked), NULL, this);
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnCancelClicked), NULL, this);

  try
  {
    if ((Package = FPackage::GetPackageNamed(packageName.ToStdWstring())))
    {
      Package->Load();
    }
  }
  catch (const std::exception& e)
  {
    REDialog::Error(e.what(), "Failed to open the package!");
    Package = nullptr;
    return;
  }

  wxDataViewColumn* col = new wxDataViewColumn(wxEmptyString, new wxDataViewIconTextRenderer, 1, wxDVC_DEFAULT_WIDTH, wxALIGN_LEFT);
  ObjectTreeCtrl->AppendColumn(col);

  LoadObjectTree();

  if (selection > 0)
  {
    if (ObjectTreeNode* node = ((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->FindItemByObjectIndex(selection))
    {
      auto item = wxDataViewItem(node);
      ObjectTreeCtrl->Select(item);
      ObjectTreeCtrl->EnsureVisible(item);
      Selection = Package->GetObject(selection);
      OkButton->Enable(Selection);
    }
  }
  else
  {
    auto root = wxDataViewItem(((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->GetRootExport());
    ObjectTreeCtrl->Expand(root);
  }
}

ObjectPicker::ObjectPicker(wxWindow* parent, const wxString& title, bool allowDifferentPackage, std::shared_ptr<FPackage> package, PACKAGE_INDEX selection, const std::vector<FString>& allowedClasses)
  : WXDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(441, 438))
{
  SetSize(FromDIP(GetSize()));
  Filter = allowedClasses;
  SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxVERTICAL);

  ContentsLabel = new wxStaticText(this, wxID_ANY, wxT("Contents:"), wxDefaultPosition, wxDefaultSize, 0);
  ContentsLabel->Wrap(-1);
  bSizer2->Add(ContentsLabel, 0, wxALL | wxEXPAND, FromDIP(5));

  ObjectTreeCtrl = new ObjectTreeDataViewCtrl(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(250, -1)), wxDV_NO_HEADER);
  bSizer2->Add(ObjectTreeCtrl, 1, wxRIGHT | wxEXPAND, 1);

  wxBoxSizer* bSizer4;
  bSizer4 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel6;
  m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(-1, 60)), wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);

  bSizer11->SetMinSize(FromDIP(wxSize(-1, 60)));
  PackageButton = new wxButton(m_panel6, wxID_ANY, wxT("Package..."), wxDefaultPosition, wxDefaultSize, 0);
  PackageButton->Enable(AllowDifferentPackage);

  bSizer11->Add(PackageButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


  bSizer11->Add(0, 0, 1, wxEXPAND, FromDIP(5));

  OkButton = new wxButton(m_panel6, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  OkButton->Enable(false);

  bSizer11->Add(OkButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

  CancelButton = new wxButton(m_panel6, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer11->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


  m_panel6->SetSizer(bSizer11);
  m_panel6->Layout();
  bSizer4->Add(m_panel6, 1, wxALL, 0);


  bSizer2->Add(bSizer4, 0, wxEXPAND, FromDIP(5));


  SetSizer(bSizer2);
  Layout();

  Centre(wxBOTH);

  // Connect Events
  ObjectTreeCtrl->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler(ObjectPicker::OnShowContextMenu), nullptr, this);
  ObjectTreeCtrl->Connect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(ObjectPicker::OnObjectSelected), nullptr, this);
  PackageButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnPackageClicked), nullptr, this);
  OkButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnOkClicked), nullptr, this);
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnCancelClicked), nullptr, this);

  Package = package;

  wxDataViewColumn* col = new wxDataViewColumn(wxEmptyString, new wxDataViewIconTextRenderer, 1, wxDVC_DEFAULT_WIDTH, wxALIGN_LEFT);
  ObjectTreeCtrl->AppendColumn(col);

  LoadObjectTree();

  if (selection > 0)
  {
    if (ObjectTreeNode* node = ((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->FindItemByObjectIndex(selection))
    {
      auto item = wxDataViewItem(node);
      ObjectTreeCtrl->Select(item);
      ObjectTreeCtrl->EnsureVisible(item);
      Selection = Package->GetObject(selection);
      OkButton->Enable(Selection);
    }
  }
  else
  {
    auto root = wxDataViewItem(((ObjectTreeModel*)ObjectTreeCtrl->GetModel())->GetRootExport());
    ObjectTreeCtrl->Expand(root);
  }
}

ObjectPicker::~ObjectPicker()
{
  ObjectTreeCtrl->Disconnect(wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler(ObjectPicker::OnObjectSelected), NULL, this);
  PackageButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnPackageClicked), NULL, this);
  OkButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnOkClicked), NULL, this);
  CancelButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectPicker::OnCancelClicked), NULL, this);
  FPackage::UnloadPackage(Package);
}

void ObjectPicker::SetCanChangePackage(bool flag)
{
  AllowDifferentPackage = flag;
  PackageButton->Enable(flag);
  UpdateTableTitle();
}

void ObjectPicker::OnObjectSelected(wxDataViewEvent& event)
{
  Selection = nullptr;
  ObjectTreeNode* node = (ObjectTreeNode*)ObjectTreeCtrl->GetCurrentItem().GetID();
  OkButton->Enable(node);
  if (!node)
  {
    return;
  }
  PACKAGE_INDEX index = node->GetObjectIndex();
  if (index == FAKE_EXPORT_ROOT && AllowRootExport)
  {
    Selection = nullptr;
    return;
  }
  if (index == FAKE_EXPORT_ROOT || index == FAKE_IMPORT_ROOT)
  {
    OkButton->Enable(false);
    return;
  }
  bool enabled = (ObjectTreeModel*)(ObjectTreeCtrl->GetModel())->IsEnabled(wxDataViewItem(node), 0);
  OkButton->Enable(enabled);
  if (enabled)
  {
    Selection = Package->GetObject(index);
  }
}

void ObjectPicker::OnPackageClicked(wxCommandEvent& event)
{
  wxMenu menu;
  menu.Append(1, wxT("Open a GPK file..."));
  menu.Append(2, wxT("Open a composite package..."))->Enable(FPackage::GetCoreVersion() == VER_TERA_MODERN);

  const int selectedMenuId = GetPopupMenuSelectionFromUser(menu);

  if (selectedMenuId == 1)
  {
    wxString path = App::GetSharedApp()->ShowOpenDialog();
    if (path.empty())
    {
      return;
    }

    try
    {
      if (auto pkg = FPackage::GetPackage(path.ToStdWstring()))
      {
        pkg->Load();
        FPackage::UnloadPackage(Package);
        Package = pkg;
      }
    }
    catch (const std::exception& e)
    {
      REDialog::Error(e.what(), "Failed to open the package!");
      return;
    }
  }
  else if (selectedMenuId == 2)
  {
    CompositePackagePicker picker(this, wxT("Open a package"));
    if (picker.ShowModal() != wxID_OK || picker.GetResult().empty())
    {
      return;
    }

    try
    {
      if (auto pkg = FPackage::GetPackageNamed(picker.GetResult().ToStdWstring()))
      {
        pkg->Load();
        FPackage::UnloadPackage(Package);
        Package = pkg;
      }
    }
    catch (const std::exception& e)
    {
      REDialog::Error(e.what(), "Failed to open the package!");
      return;
    }
  }
  else
  {
    return;
  }
  
  Selection = nullptr;
  LoadObjectTree();
}

void ObjectPicker::OnOkClicked(wxCommandEvent& event)
{
  EndModal(wxID_OK);
}

void ObjectPicker::OnCancelClicked(wxCommandEvent& event)
{
  EndModal(wxID_CANCEL);
}

void ObjectPicker::LoadObjectTree()
{
  if (!Package)
  {
    return;
  }
  ObjectTreeModel* model = new ObjectTreeModel(Package->GetPackageName(), Package->GetRootExports(), std::vector<FObjectImport*>(), Filter);
  model->GetRootExport()->SetCustomObjectIndex(FAKE_EXPORT_ROOT);
  ObjectTreeCtrl->AssociateModel(model);
  ObjectTreeCtrl->GetColumn(0)->SetWidth(ObjectTreeCtrl->GetSize().x - 4);
  ObjectTreeCtrl->ExpandAll();
  model->DecRef();
  Selection = nullptr;
  OkButton->Enable(false);
  UpdateTableTitle();
}

void ObjectPicker::UpdateTableTitle()
{
  if (!ObjectTreeCtrl->HasFilter() || ObjectTreeCtrl->SuitableObjectsCount())
  {
    ContentsLabel->SetLabel("Contents:");
    ContentsLabel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_CAPTIONTEXT));
  }
  else
  {
    wxString text("No suitable objects found!");
    if (AllowDifferentPackage)
    {
      text += wxT(" Press \"Package...\" to open a different GPK.");
    }
    ContentsLabel->SetLabel(text);
    ContentsLabel->SetForegroundColour(wxColour(255, 0, 0));
  }
}

void ObjectPicker::OnShowContextMenu(wxDataViewEvent& event)
{
  if (!event.GetItem().IsOk() || !AllowNewPackage)
  {
    return;
  }
  ObjectTreeNode* node = (ObjectTreeNode*)ObjectTreeCtrl->GetCurrentItem().GetID();
  if (!node)
  {
    return;
  }

  wxMenu menu;
  menu.Append(1, wxT("Create a package..."));

  if (!GetPopupMenuSelectionFromUser(menu))
  {
    return;
  }

  FObjectExport* exp = node->GetObjectIndex() == FAKE_EXPORT_ROOT ? nullptr : Package->GetObject(node->GetObjectIndex())->GetExportObject();
  ObjectNameDialog dlg(this, wxT("Untitled"));
  dlg.SetValidator(ObjectNameDialog::GetDefaultValidator(exp, Package.get()));
  if (dlg.ShowModal() != wxID_OK)
  {
    return;
  }
  wxString name = dlg.GetObjectName();
  if (name.empty())
  {
    return;
  }

  if (FObjectExport* newExp = Package->AddExport(FString(name.ToStdWstring()), NAME_Package, exp))
  {
    ObjectTreeCtrl->AddExportObject(newExp);
    wxDataViewEvent tmp;
    OnObjectSelected(tmp);
  }
}

ObjectNameDialog::Validator ObjectNameDialog::GetDefaultValidator(FObjectExport* parent, FPackage* package)
{
  return [=](const wxString& name) {
    if (!package)
    {
      return false;
    }
    if (name.Find(" ") != wxString::npos)
    {
      return false;
    }
    FString tmpName = name.ToStdWstring();
    if (!tmpName.IsAnsi())
    {
      return false;
    }
    std::vector<FObjectExport*> exps;
    if (parent)
    {
      exps = parent->Inner;
    }
    else
    {
      exps = package->GetRootExports();
    }
    for (FObjectExport* exp : exps)
    {
      if (exp->GetObjectName() == tmpName)
      {
        return false;
      }
    }
    return true;
  };
}

ObjectNameDialog::ObjectNameDialog(wxWindow* parent, const wxString& objectName)
  : WXDialog(parent, wxID_ANY, wxT("Enter object name"), wxDefaultPosition, wxSize(418, 143), wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU)
{
  SetSize(FromDIP(GetSize()));
  SetSizeHints(wxDefaultSize, wxDefaultSize);
  wxBoxSizer* bSizer20;
  bSizer20 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer21;
  bSizer21 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText10;
  m_staticText10 = new wxStaticText(this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText10->Wrap(-1);
  bSizer21->Add(m_staticText10, 0, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));

  NameField = new wxTextCtrl(this, wxID_ANY, objectName, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  NameField->SetMaxLength(1024);

  bSizer21->Add(NameField, 1, wxALL | wxALIGN_CENTER_VERTICAL, FromDIP(5));


  bSizer20->Add(bSizer21, 1, wxEXPAND, FromDIP(5));

  wxBoxSizer* bSizer22;
  bSizer22 = new wxBoxSizer(wxHORIZONTAL);


  bSizer22->Add(0, 0, 1, wxEXPAND, FromDIP(5));

  OkButton = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer22->Add(OkButton, 0, wxALL, FromDIP(5));

  CancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer22->Add(CancelButton, 0, wxALL, FromDIP(5));


  bSizer20->Add(bSizer22, 0, wxEXPAND, FromDIP(5));


  this->SetSizer(bSizer20);
  this->Layout();

  this->Centre(wxBOTH);

  OkButton->Enable(NameField->GetValue().size());
  NameField->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ObjectNameDialog::OnName), nullptr, this);
  NameField->Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(ObjectNameDialog::OnNameEnter), nullptr, this);
  OkButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectNameDialog::OnOkClicked), nullptr, this);
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectNameDialog::OnCancelClicked), nullptr, this);
}

ObjectNameDialog::~ObjectNameDialog()
{
  NameField->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ObjectNameDialog::OnName), nullptr, this);
  NameField->Disconnect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(ObjectNameDialog::OnNameEnter), nullptr, this);
  OkButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectNameDialog::OnOkClicked), nullptr, this);
  CancelButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ObjectNameDialog::OnCancelClicked), nullptr, this);
}

void ObjectNameDialog::SetValidator(Validator& validator)
{
  ValidatorFunc = validator;
}

wxString ObjectNameDialog::GetObjectName() const
{
  return NameField->GetValue();
}

void ObjectNameDialog::OnName(wxCommandEvent&)
{
  OkButton->Enable(NameField->GetValue().size());
}

void ObjectNameDialog::OnNameEnter(wxCommandEvent& e)
{
  OkButton->Enable(NameField->GetValue().size());
  if (OkButton->IsEnabled())
  {
    OnOkClicked(e);
  }
}

void ObjectNameDialog::OnOkClicked(wxCommandEvent&)
{
  if (ValidatorFunc && !ValidatorFunc(GetObjectName()))
  {
    REDialog::Warning(wxT("Can't create object with this name."));
    return;
  }
  EndModal(wxID_OK);
}

void ObjectNameDialog::OnCancelClicked(wxCommandEvent&)
{
  EndModal(wxID_CANCEL);
}

ClassPicker::ClassPicker(wxWindow* parent, const wxString& title) 
  : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(393, 177), wxDEFAULT_DIALOG_STYLE)
{
  SetSize(FromDIP(GetSize()));
  SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText1;
  m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("Class:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText1->Wrap(-1);
  bSizer2->Add(m_staticText1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  ClassCombo = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_SORT | wxTE_PROCESS_ENTER);
  bSizer2->Add(ClassCombo, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  ClassCombo->Freeze();

  std::vector<UClass*> allClasses = FPackage::GetClasses();

  for (UClass* cls : allClasses)
  {
    ClassCombo->Append(cls->GetObjectName().String().WString());
  }

  ClassCombo->Thaw();


  bSizer1->Add(bSizer2, 1, wxEXPAND, 5);

  wxStaticLine* m_staticline1;
  m_staticline1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  bSizer1->Add(m_staticline1, 0, wxEXPAND | wxTOP | wxBOTTOM, 5);

  wxPanel* m_panel1;
  m_panel1 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 35), wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer3;
  bSizer3 = new wxBoxSizer(wxHORIZONTAL);


  bSizer3->Add(0, 0, 1, wxEXPAND, 5);

  OkButton = new wxButton(m_panel1, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer3->Add(OkButton, 0, wxALL, 5);

  CancelButton = new wxButton(m_panel1, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer3->Add(CancelButton, 0, wxALL, 5);


  m_panel1->SetSizer(bSizer3);
  m_panel1->Layout();
  bSizer1->Add(m_panel1, 0, wxEXPAND | wxALL, 5);


  this->SetSizer(bSizer1);
  this->Layout();

  this->Centre(wxBOTH);

  OkButton->Enable(false);
  ClassCombo->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(ClassPicker::OnClassSelected), NULL, this);
  ClassCombo->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(ClassPicker::OnClassText), NULL, this);
  ClassCombo->Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(ClassPicker::OnClassTextEnter), NULL, this);
  OkButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ClassPicker::OnOkClicked), NULL, this);
  CancelButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ClassPicker::OnCancelClicked), NULL, this);
}

wxString ClassPicker::GetSelectedClassName() const
{
  return OkButton->IsEnabled() ? ClassCombo->GetValue() : wxEmptyString;
}

void ClassPicker::OnClassSelected(wxCommandEvent&)
{
  OkButton->Enable(true);
}

void ClassPicker::OnClassText(wxCommandEvent&)
{
  OkButton->Enable(FPackage::FindClass(ClassCombo->GetValue().ToStdWstring()));
}

void ClassPicker::OnClassTextEnter(wxCommandEvent&)
{
  OkButton->Enable(FPackage::FindClass(ClassCombo->GetValue().ToStdWstring()));
  if (OkButton->IsEnabled())
  {
    EndModal(wxID_OK);
  }
}

void ClassPicker::OnOkClicked(wxCommandEvent&)
{
  EndModal(wxID_OK);
}

void ClassPicker::OnCancelClicked(wxCommandEvent&)
{
  EndModal(wxID_CANCEL);
}
