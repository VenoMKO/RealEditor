#include "ObjectPicker.h"
#include "../App.h"
#include "../Misc/ObjectTreeModel.h"
#include "CompositePackagePicker.h"
#include "REDialogs.h"

#include <Tera/FPackage.h>
#include <Tera/FObjectResource.h>
#include <Tera/UObject.h>

ObjectPicker::ObjectPicker(wxWindow* parent, const wxString& title, bool allowDifferentPackage, const wxString& packageName, PACKAGE_INDEX selection, const std::vector<FString>& allowedClasses)
  : WXDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(441, 438))
  , AllowDifferentPackage(allowDifferentPackage)
{
  Filter = allowedClasses;
  SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxVERTICAL);

  ObjectTreeCtrl = new ObjectTreeDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(250, -1), 0);
  bSizer2->Add(ObjectTreeCtrl, 1, wxRIGHT | wxEXPAND, 1);

  wxBoxSizer* bSizer4;
  bSizer4 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel6;
  m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 60), wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);

  bSizer11->SetMinSize(wxSize(-1, 60));
  PackageButton = new wxButton(m_panel6, wxID_ANY, wxT("Package..."), wxDefaultPosition, wxDefaultSize, 0);
  PackageButton->Enable(AllowDifferentPackage);

  bSizer11->Add(PackageButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  bSizer11->Add(0, 0, 1, wxEXPAND, 5);

  OkButton = new wxButton(m_panel6, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  OkButton->Enable(false);

  bSizer11->Add(OkButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  CancelButton = new wxButton(m_panel6, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer11->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  m_panel6->SetSizer(bSizer11);
  m_panel6->Layout();
  bSizer4->Add(m_panel6, 1, wxALL, 0);


  bSizer2->Add(bSizer4, 0, wxEXPAND, 5);


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
  Filter = allowedClasses;
  SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxVERTICAL);

  ContentsLabel = new wxStaticText(this, wxID_ANY, wxT("Contents:"), wxDefaultPosition, wxDefaultSize, 0);
  ContentsLabel->Wrap(-1);
  bSizer2->Add(ContentsLabel, 0, wxALL | wxEXPAND, 5);

  ObjectTreeCtrl = new ObjectTreeDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(250, -1), wxDV_NO_HEADER);
  bSizer2->Add(ObjectTreeCtrl, 1, wxRIGHT | wxEXPAND, 1);

  wxBoxSizer* bSizer4;
  bSizer4 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel6;
  m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 60), wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxHORIZONTAL);

  bSizer11->SetMinSize(wxSize(-1, 60));
  PackageButton = new wxButton(m_panel6, wxID_ANY, wxT("Package..."), wxDefaultPosition, wxDefaultSize, 0);
  PackageButton->Enable(AllowDifferentPackage);

  bSizer11->Add(PackageButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  bSizer11->Add(0, 0, 1, wxEXPAND, 5);

  OkButton = new wxButton(m_panel6, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  OkButton->Enable(false);

  bSizer11->Add(OkButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  CancelButton = new wxButton(m_panel6, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer11->Add(CancelButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  m_panel6->SetSizer(bSizer11);
  m_panel6->Layout();
  bSizer4->Add(m_panel6, 1, wxALL, 0);


  bSizer2->Add(bSizer4, 0, wxEXPAND, 5);


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
  PackageButton->Enable(flag);
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
  model->DecRef();
  if (Filter.size())
  {
    if (ObjectTreeCtrl->SuitableObjectsCount())
    {
      ContentsLabel->SetLabel("Contents:");
      ContentsLabel->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_CAPTIONTEXT));
    }
    else
    {
      wxString text("No suitable objects found!");
      if (PackageButton->IsEnabled())
      {
        text += wxT(" Press \"Package...\" to open a different GPK.");
      }
      ContentsLabel->SetLabel(text);
      ContentsLabel->SetForegroundColour(wxColour(255, 0, 0));
    }
    ObjectTreeCtrl->ExpandAll();
  }
  Selection = nullptr;
  OkButton->Enable(false);
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
  SetSizeHints(wxDefaultSize, wxDefaultSize);
  wxBoxSizer* bSizer20;
  bSizer20 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer21;
  bSizer21 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText10;
  m_staticText10 = new wxStaticText(this, wxID_ANY, wxT("Name:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText10->Wrap(-1);
  bSizer21->Add(m_staticText10, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  NameField = new wxTextCtrl(this, wxID_ANY, objectName, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  NameField->SetMaxLength(1024);

  bSizer21->Add(NameField, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  bSizer20->Add(bSizer21, 1, wxEXPAND, 5);

  wxBoxSizer* bSizer22;
  bSizer22 = new wxBoxSizer(wxHORIZONTAL);


  bSizer22->Add(0, 0, 1, wxEXPAND, 5);

  OkButton = new wxButton(this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer22->Add(OkButton, 0, wxALL, 5);

  CancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer22->Add(CancelButton, 0, wxALL, 5);


  bSizer20->Add(bSizer22, 0, wxEXPAND, 5);


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
