#include "WelcomeDialog.h"
#include "DcToolDialog.h"
#include "SettingsWindow.h"
#include "ProgressWindow.h"
#include "IODialogs.h"
#include "../App.h"

#include <wx/statline.h>

#include <Tera/Core.h>
#include <Tera/FPackage.h>

struct RecentItem {
  wxString Name;
  FString Path;
};

class RecentModel : public wxDataViewListModel {
public:
  RecentModel(const std::vector<FString>& recent)
  {
    Recent.reserve(recent.size());
    for (const FString& path : recent)
    {
      RecentItem item;
      if (path.StartWith("composite\\"))
      {
        item.Name = path.Substr(10).WString();
      }
      else if (path.StartWith("named\\"))
      {
        item.Name = path.Substr(6).WString();
      }
      else
      {
        item.Name = path.Filename(false).WString();
      }
      item.Path = path;

      for (RecentItem& recentItem : Recent)
      {
        if (recentItem.Name == item.Name)
        {
          item.Name = path.WString();
          recentItem.Name = recentItem.Path.WString();
        }
      }
      Recent.emplace_back(item);
    }
    IconList = new wxImageList(16, 16, true, 2);
    IconList->Add(wxBitmap("#130", wxBITMAP_TYPE_PNG_RESOURCE));
  }

  ~RecentModel()
  {
    delete IconList;
  }

  unsigned int GetColumnCount() const override
  {
    return 1;
  }

  bool HasValue(const wxDataViewItem& item, unsigned col) const override
  {
    return Recent.size();
  }

  void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override
  {
    RecentItem* r = (RecentItem*)item.GetID();
    wxDataViewIconText itxt(r->Name);
    itxt.SetIcon(IconList->GetIcon(0));
    variant << itxt;
  }

  bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col) override
  {
    return false;
  }

  wxString GetColumnType(unsigned int col) const override
  {
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
      for (const auto& r : Recent)
      {
        array.Add(wxDataViewItem((void*)&r));
      }
      return Recent.size();
    }
    return 0;
  }

  unsigned int GetCount() const override
  {
    return Recent.size();
  }

  unsigned int GetRow(const wxDataViewItem& item) const override
  {
    for (int32 idx = 0; idx < Recent.size(); ++idx)
    {
      if (&Recent[idx] == (RecentItem*)item.GetID())
      {
        return idx;
      }
    }
    return 0;
  }

  void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override
  {
    wxDataViewIconText itxt(Recent[row].Name);
    variant << itxt;
  }

  bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override
  {
    return false;
  }

protected:
  std::vector<RecentItem> Recent;
  wxImageList* IconList = nullptr;
};

WelcomeDialog::WelcomeDialog(wxWindow* parent, bool startMode)
  : wxDialog(parent, wxID_ANY, wxT("Real Editor x64"), wxDefaultPosition, wxSize(729, 387), wxDEFAULT_DIALOG_STYLE)
{
  SetIcon(wxICON(#114));
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer2;
  bSizer2 = new wxBoxSizer(wxHORIZONTAL);

  RecentCtrl = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(230, -1), 0);
  bSizer2->Add(RecentCtrl, 0, wxALL | wxEXPAND, 1);

  wxBoxSizer* bSizer3;
  bSizer3 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText1;
  m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("Common actions"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText1->Wrap(-1);
  m_staticText1->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer3->Add(m_staticText1, 0, wxTOP | wxRIGHT | wxLEFT, 5);

  wxPanel* m_panel3;
  m_panel3 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer5;
  bSizer5 = new wxBoxSizer(wxVERTICAL);

  wxPanel* m_panel4;
  m_panel4 = new wxPanel(m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  bSizer5->Add(m_panel4, 0, wxEXPAND | wxALL, 6);

  wxBoxSizer* bSizer4;
  bSizer4 = new wxBoxSizer(wxHORIZONTAL);

  OpenFileButton = new wxButton(m_panel3, wxID_ANY, wxT("Open package..."), wxDefaultPosition, wxSize(115, -1), 0);
  OpenFileButton->SetToolTip(wxT("Open a GPK, GMP, U file from a disk"));

  bSizer4->Add(OpenFileButton, 0, wxALL, 5);

  wxStaticText* m_staticText13;
  m_staticText13 = new wxStaticText(m_panel3, wxID_ANY, wxT("Open a GPK/GMP/U file"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText13->Wrap(-1);
  bSizer4->Add(m_staticText13, 0, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);


  bSizer4->Add(0, 0, 1, wxEXPAND, 5);


  bSizer5->Add(bSizer4, 0, wxEXPAND, 5);

  wxBoxSizer* bSizer18;
  bSizer18 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText3;
  m_staticText3 = new wxStaticText(m_panel3, wxID_ANY, wxT("Open by name:"), wxDefaultPosition, wxSize(110, -1), wxALIGN_RIGHT);
  m_staticText3->Wrap(-1);
  bSizer18->Add(m_staticText3, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

  OpenByNameField = new wxTextCtrl(m_panel3, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, -1), wxTE_PROCESS_ENTER);
  bSizer18->Add(OpenByNameField, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  OpenByNameButton = new wxButton(m_panel3, wxID_ANY, wxT("Open"), wxDefaultPosition, wxDefaultSize, 0);
  OpenByNameButton->SetToolTip(wxT("Open a package by name"));

  bSizer18->Add(OpenByNameButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  bSizer5->Add(bSizer18, 1, wxEXPAND, 5);

  wxStaticLine* m_staticline1;
  m_staticline1 = new wxStaticLine(m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  bSizer5->Add(m_staticline1, 0, wxEXPAND | wxTOP | wxBOTTOM, 15);

  wxBoxSizer* bSizer10;
  bSizer10 = new wxBoxSizer(wxHORIZONTAL);

  ObjectDumpButton = new wxButton(m_panel3, wxID_ANY, wxT("Object dump..."), wxDefaultPosition, wxSize(115, -1), 0);
  bSizer10->Add(ObjectDumpButton, 0, wxALL, 5);

  wxStaticText* m_staticText6;
  m_staticText6 = new wxStaticText(m_panel3, wxID_ANY, wxT("Generate a composite object dump(ObjectDump.txt)"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText6->Wrap(-1);
  bSizer10->Add(m_staticText6, 1, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);


  bSizer5->Add(bSizer10, 1, wxEXPAND, 5);

  wxBoxSizer* bSizer15;
  bSizer15 = new wxBoxSizer(wxHORIZONTAL);

  DataCenterButton = new wxButton(m_panel3, wxID_ANY, wxT("DataCenter Tool"), wxDefaultPosition, wxSize(115, -1), 0);
  bSizer15->Add(DataCenterButton, 0, wxALL | wxEXPAND, 5);

  wxStaticText* m_staticText61;
  m_staticText61 = new wxStaticText(m_panel3, wxID_ANY, wxT("Unpack and export DataCenter_Final.dat file"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText61->Wrap(-1);
  bSizer15->Add(m_staticText61, 1, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);


  bSizer5->Add(bSizer15, 1, wxEXPAND, 5);

  wxBoxSizer* bSizer16;
  bSizer16 = new wxBoxSizer(wxHORIZONTAL);

  SettingsButton = new wxButton(m_panel3, wxID_ANY, wxT("Settings"), wxDefaultPosition, wxSize(115, -1), 0);
  bSizer16->Add(SettingsButton, 0, wxALL | wxEXPAND, 5);

  wxStaticText* m_staticText10;
  m_staticText10 = new wxStaticText(m_panel3, wxID_ANY, wxT("Real Editor settings"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText10->Wrap(-1);
  bSizer16->Add(m_staticText10, 0, wxTOP | wxBOTTOM | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);


  bSizer5->Add(bSizer16, 1, wxEXPAND, 5);

  wxPanel* m_panel41;
  m_panel41 = new wxPanel(m_panel3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  bSizer5->Add(m_panel41, 0, wxEXPAND | wxALL, 5);


  m_panel3->SetSizer(bSizer5);
  m_panel3->Layout();
  bSizer5->Fit(m_panel3);
  bSizer3->Add(m_panel3, 0, wxEXPAND | wxALL, 5);


  bSizer3->Add(0, 0, 1, wxEXPAND, 5);

  wxBoxSizer* bSizer14;
  bSizer14 = new wxBoxSizer(wxHORIZONTAL);

  UpdateButton = new wxButton(this, wxID_ANY, wxT("Update"), wxDefaultPosition, wxSize(115, -1), 0);
  bSizer14->Add(UpdateButton, 0, wxTOP | wxBOTTOM | wxLEFT, 12);

  Version = new wxStaticText(this, wxID_ANY, GetAppVersion(), wxDefaultPosition, wxDefaultSize, 0);
  Version->Wrap(-1);
  Version->SetToolTip(wxString::Format("Build: %u", BUILD_NUMBER));
  bSizer14->Add(Version, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  bSizer14->Add(0, 0, 1, wxEXPAND, 5);

  CloseButton = new wxButton(this, wxID_ANY, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer14->Add(CloseButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


  bSizer3->Add(bSizer14, 0, wxEXPAND, 5);


  bSizer2->Add(bSizer3, 1, wxEXPAND, 5);


  bSizer1->Add(bSizer2, 1, wxEXPAND, 5);


  this->SetSizer(bSizer1);
  this->Layout();

  this->Centre(wxBOTH);

  RecentCtrl->Connect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(WelcomeDialog::OnRecentActivated), NULL, this);
  OpenFileButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnOpenFileClicked), NULL, this);
  OpenByNameField->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(WelcomeDialog::OnOpenByNameText), NULL, this);
  OpenByNameField->Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(WelcomeDialog::OnOpenByNameEnter), NULL, this);
  OpenByNameButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnOpenByNameClicked), NULL, this);
  ObjectDumpButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnObjectDumpClicked), NULL, this);
  DataCenterButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnDataCenterClicked), NULL, this);
  SettingsButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnSettingsClicked), NULL, this);
  UpdateButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnUpdateClicked), NULL, this);
  CloseButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnCloseClicked), NULL, this);

  const auto& lastItems = App::GetSharedApp()->GetConfig().LastFilePackages;

  OpenByNameButton->Enable(false);
  RecentCtrl->AppendIconTextColumn(wxT("Recent"), 0, wxDATAVIEW_CELL_INERT, 210, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE);

  RecentModel* model = new RecentModel(lastItems);
  RecentCtrl->AssociateModel(model);
  model->DecRef();

  StartMode = startMode;
}

WelcomeDialog::~WelcomeDialog()
{
  RecentCtrl->Disconnect(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler(WelcomeDialog::OnRecentActivated), NULL, this);
  OpenFileButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnOpenFileClicked), NULL, this);
  OpenByNameField->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(WelcomeDialog::OnOpenByNameText), NULL, this);
  OpenByNameField->Disconnect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(WelcomeDialog::OnOpenByNameEnter), NULL, this);
  OpenByNameButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnOpenByNameClicked), NULL, this);
  ObjectDumpButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnObjectDumpClicked), NULL, this);
  DataCenterButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnDataCenterClicked), NULL, this);
  SettingsButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnSettingsClicked), NULL, this);
  UpdateButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnUpdateClicked), NULL, this);
  CloseButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(WelcomeDialog::OnCloseClicked), NULL, this);
}

void WelcomeDialog::OnExternalOpen(const wxString& path)
{
  QueuedOpenList.emplace_back(path);
  if (!ModalRunning)
  {
    EndModal(wxID_OK);
  }
}

int WelcomeDialog::ShowModal()
{
  std::vector<FString> pkgList = FPackage::FilePackageNames;
  if (pkgList.empty())
  {
    pkgList = FPackage::GetCachedDirCache(App::GetSharedApp()->GetConfig().RootDir);
    for (FString& path : pkgList)
    {
      path = path.Filename(false);
    }
    if (pkgList.empty())
    {
      ModalRunning = true;
      ProgressWindow progress(nullptr, "Loading...");
      progress.SetActionText("Enumerating S1Game contents...");
      progress.SetCurrentProgress(-1);
      progress.SetCanCancel(false);
      std::vector<std::pair<std::string, std::string>> failed;
      std::thread([&] {
        FPackage::UpdateDirCache(App::GetSharedApp()->GetConfig().RootDir);
        pkgList = FPackage::FilePackageNames;
        SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      }).detach();
      progress.ShowModal();
      ModalRunning = false;
    }
  }
  if (pkgList.size())
  {
    wxArrayString autoComplete;
    autoComplete.reserve(pkgList.size());
    for (const FString& name : pkgList)
    {
      autoComplete.push_back(name.WString());
    }
    OpenByNameField->AutoComplete(autoComplete);
    OpenByNameField->SetFocus();
  }
  else
  {
    OpenByNameField->Enable(false);
    OpenByNameButton->Enable(false);
  }
  return wxDialog::ShowModal();
}

void WelcomeDialog::OnRecentActivated(wxDataViewEvent& event)
{
  RecentItem* item = (RecentItem*)event.GetItem().GetID();
  if (!item)
  {
    return;
  }
  QueuedOpenList.emplace_back(item->Path.WString());
  EndModal(wxID_OK);
}

void WelcomeDialog::OnOpenFileClicked(wxCommandEvent& event)
{
  ModalRunning = true;
  wxString path = IODialog::OpenPackageDialog(this);
  ModalRunning = false;
  if (path.size())
  {
    QueuedOpenList.emplace_back(path);
  }
  if (QueuedOpenList.size())
  {
    EndModal(wxID_OK);
  }
}

void WelcomeDialog::OnOpenByNameText(wxCommandEvent& event)
{
  OpenByNameButton->Enable(OpenByNameField->GetValue().size());
}

void WelcomeDialog::OnOpenByNameEnter(wxCommandEvent& event)
{

  if (OpenByNameButton->IsEnabled())
  {
    OnOpenByNameClicked(event);
  }
}

void WelcomeDialog::OnOpenByNameClicked(wxCommandEvent& event)
{
  QueuedOpenList.emplace_back(wxS("named\\") + OpenByNameField->GetValue());
  EndModal(wxID_OK);
}

void WelcomeDialog::OnObjectDumpClicked(wxCommandEvent& event)
{
  QueuedOpenList.emplace_back("OBJDUMP");
  EndModal(wxID_OK);
}

void WelcomeDialog::OnDataCenterClicked(wxCommandEvent& event)
{
  ModalRunning = true;
  DcToolDialog dlg(nullptr);
  dlg.ShowModal();
  ModalRunning = false;
  if (QueuedOpenList.size())
  {
    EndModal(wxID_OK);
  }
}

void WelcomeDialog::OnSettingsClicked(wxCommandEvent& event)
{
  ModalRunning = true;
  FAppConfig cfg;
  SettingsWindow dlg(App::GetSharedApp()->GetConfig(), cfg, false);
  if (dlg.ShowModal() == wxID_OK)
  {
    App::GetSharedApp()->GetConfig() = cfg;
  }
  ModalRunning = false;
  if (QueuedOpenList.size())
  {
    EndModal(wxID_OK);
  }
}

void WelcomeDialog::OnUpdateClicked(wxCommandEvent& event)
{
  SettingsWindow::OpenUpdateLink();
}

void WelcomeDialog::OnCloseClicked(wxCommandEvent& event)
{
  QueuedOpenList.clear();
  EndModal(wxID_CANCEL);
}
