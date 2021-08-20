#include "SettingsWindow.h"
#include "ProgressWindow.h"
#include "DcToolDialog.h"
#include "REDialogs.h"
#include "../App.h"

#include <wx/statline.h>
#include <thread>

#include <Tera/FPackage.h>

const wxString RootDir = wxS("S1Game");
const wxString UpdatesUrl = wxS("https://github.com/VenoMKO/RealEditor/releases");

enum ControlElementId {
  Path = wxID_HIGHEST + 1,
  Browse,
  DirCache,
  Mappers,
  Warnings,
  Register,
  Unregister,
  Update,
  DcTool,
  FastDump,
};

bool IsValidDir(const wxString& path)
{
#if BNS
  return true;
#else
  return path.EndsWith(wxFILE_SEP_PATH + RootDir) || path.EndsWith(wxFILE_SEP_PATH + RootDir + wxFILE_SEP_PATH);
#endif
}

SettingsWindow::SettingsWindow(const FAppConfig& currentConfig, FAppConfig& output, bool allowRebuild, const wxPoint& pos)
  : WXDialog(nullptr, wxID_ANY, wxS("Settings"), pos, wxSize(668, 448), wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU | wxTAB_TRAVERSAL)
  , CurrentConfig(currentConfig)
  , NewConfig(output)
  , AllowRebuild(allowRebuild)
{
  NewConfig = currentConfig;
  SetIcon(wxICON(#114));
  this->SetSizeHints(wxDefaultSize, wxDefaultSize);
  this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

  wxBoxSizer* bSizer1;
  bSizer1 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText5;
  m_staticText5 = new wxStaticText(this, wxID_ANY, wxT("S1Game"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText5->Wrap(-1);
  m_staticText5->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer1->Add(m_staticText5, 0, wxALL, 5);

  wxPanel* m_panel2;
  m_panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer4;
  bSizer4 = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer* bSizer5;
  bSizer5 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText7;
  m_staticText7 = new wxStaticText(m_panel2, wxID_ANY, wxT("Path:"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText7->Wrap(-1);
  bSizer5->Add(m_staticText7, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxString root = CurrentConfig.UseBuiltInS1Game32 ? wxString("N/A") : wxString(CurrentConfig.RootDir.WString());
  PathField = new wxTextCtrl(m_panel2, ControlElementId::Path, root, wxDefaultPosition, wxSize(500, -1), wxTE_PROCESS_ENTER);
  bSizer5->Add(PathField, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  PathField->Enable(!CurrentConfig.UseBuiltInS1Game32);

  BrowseButton = new wxButton(m_panel2, ControlElementId::Browse, wxT("Browse..."), wxDefaultPosition, wxDefaultSize, 0);
  bSizer5->Add(BrowseButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  BrowseButton->Enable(!CurrentConfig.UseBuiltInS1Game32);


  bSizer4->Add(bSizer5, 1, wxEXPAND, 5);

  wxStaticText* m_staticText6;
  m_staticText6 = new wxStaticText(m_panel2, wxID_ANY, wxT("Select the S1Game folder of your Tera client. This path will be used to find and load packages and resources. The path may look like this: \"D:\\Games\\Gameforge\\Tera\\Client\\S1Game\\\"."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText6->Wrap(600);
  bSizer4->Add(m_staticText6, 0, wxBOTTOM | wxLEFT | wxRIGHT, 5);

  wxStaticLine* m_staticline2;
  m_staticline2 = new wxStaticLine(m_panel2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  bSizer4->Add(m_staticline2, 0, wxEXPAND | wxTOP | wxBOTTOM, 7);

  UseBuiltInS1Game32 = new wxCheckBox(m_panel2, wxID_ANY, wxT("Use built-in 32-bit S1Game folder"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer4->Add(UseBuiltInS1Game32, 0, wxALL, 5);

  wxStaticText* m_staticText81;
  m_staticText81 = new wxStaticText(m_panel2, wxID_ANY, wxT("Enable this option if you want to open a 32-bit GPK file, but you don't have a full 32-bit client."), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText81->Wrap(-1);
  bSizer4->Add(m_staticText81, 0, wxBOTTOM | wxRIGHT | wxLEFT, 5);

  m_panel2->SetSizer(bSizer4);
  m_panel2->Layout();
  bSizer4->Fit(m_panel2);
  bSizer1->Add(m_panel2, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer* bSizer10;
  bSizer10 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel7;
  m_panel7 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer12;
  bSizer12 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText10;
  m_staticText10 = new wxStaticText(m_panel7, wxID_ANY, wxT("Utils"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText10->Wrap(-1);
  m_staticText10->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer12->Add(m_staticText10, 0, wxALL, 5);

  wxPanel* m_panel8;
  m_panel8 = new wxPanel(m_panel7, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer13;
  bSizer13 = new wxBoxSizer(wxVERTICAL);

  RebuildCacheButton = new wxButton(m_panel8, ControlElementId::DirCache, wxT("Rebuild Cache"), wxDefaultPosition, wxDefaultSize, 0);
  RebuildCacheButton->SetToolTip(wxT("Update a cached list of packages in your S1Game folder. Usefull when ") + wxTheApp->GetAppDisplayName() + wxT(" can't find an object."));
  bSizer13->Add(RebuildCacheButton, 0, wxALL | wxEXPAND, 5);

  UpdateMappingButton = new wxButton(m_panel8, ControlElementId::Mappers, wxT("Update Mapping"), wxDefaultPosition, wxDefaultSize, 0);
  UpdateMappingButton->SetToolTip(wxT("Decrypt and update mappers. May be usefull after Tera update."));
  bSizer13->Add(UpdateMappingButton, 0, wxALL | wxEXPAND, 5);

  ResetWarningsButton = new wxButton(m_panel8, ControlElementId::Warnings, wxT("Reset Warnings"), wxDefaultPosition, wxDefaultSize, 0);
  ResetWarningsButton->SetToolTip(wxT("Reset all hidden UI warnings."));
  bSizer13->Add(ResetWarningsButton, 0, wxALL | wxEXPAND, 5);


  m_panel8->SetSizer(bSizer13);
  m_panel8->Layout();
  bSizer13->Fit(m_panel8);
  bSizer12->Add(m_panel8, 0, wxEXPAND | wxALL, 5);


  m_panel7->SetSizer(bSizer12);
  m_panel7->Layout();
  bSizer12->Fit(m_panel7);
  bSizer10->Add(m_panel7, 1, wxALL, 5);

  wxPanel* m_panel6;
  m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer11;
  bSizer11 = new wxBoxSizer(wxVERTICAL);

  wxStaticText* m_staticText8;
  m_staticText8 = new wxStaticText(m_panel6, wxID_ANY, wxT("Associate filetypes"), wxDefaultPosition, wxDefaultSize, 0);
  m_staticText8->Wrap(-1);
  m_staticText8->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

  bSizer11->Add(m_staticText8, 0, wxALL, 5);

  wxPanel* m_panel3;
  m_panel3 = new wxPanel(m_panel6, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), wxBORDER_THEME | wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer6;
  bSizer6 = new wxBoxSizer(wxVERTICAL);

  wxPanel* m_panel5;
  m_panel5 = new wxPanel(m_panel3, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer7;
  bSizer7 = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText* m_staticText9;
  m_staticText9 = new wxStaticText(m_panel5, wxID_ANY, wxT("This allows you to open *.gpk, *.gmp, *.upk and *.u packages by double clicking in the Explorer window. Press Associate to enable the feature or Dissociate to disable."), wxDefaultPosition, wxSize(-1, -1), 0);
  m_staticText9->Wrap(300);
  bSizer7->Add(m_staticText9, 0, wxALL, 5);

  wxBoxSizer* bSizer9;
  bSizer9 = new wxBoxSizer(wxVERTICAL);

  WasRegistered = ((App*)wxTheApp)->CheckMimeTypes(true);

  RegisterButton = new wxButton(m_panel5, ControlElementId::Register, wxT("Associate"), wxDefaultPosition, wxDefaultSize, 0);
  RegisterButton->SetToolTip(wxT("Allow to open packages in the Explorer by double clicking them."));
  RegisterButton->Enable(!WasRegistered);
  bSizer9->Add(RegisterButton, 0, wxALL, 5);

  UnregisterButton = new wxButton(m_panel5, ControlElementId::Unregister, wxT("Dissociate"), wxDefaultPosition, wxDefaultSize, 0);
  UnregisterButton->SetToolTip(wxT("Don't open packages in the Explorer by double clicking them."));
  UnregisterButton->Enable(WasRegistered);
  bSizer9->Add(UnregisterButton, 0, wxALL, 5);


  bSizer7->Add(bSizer9, 1, wxEXPAND, 5);


  m_panel5->SetSizer(bSizer7);
  m_panel5->Layout();
  bSizer7->Fit(m_panel5);
  bSizer6->Add(m_panel5, 0, wxALL, 5);

  m_panel3->SetSizer(bSizer6);
  m_panel3->Layout();
  bSizer6->Fit(m_panel3);
  bSizer11->Add(m_panel3, 0, wxALL, 5);

  FastObjDump = new wxCheckBox(m_panel6, ControlElementId::FastDump, wxT("Reduced ObjectDump"));
  FastObjDump->SetToolTip(wxT("Generate small ObjectDump.txt by skipping Components and UPackages."));
  FastObjDump->SetValue(CurrentConfig.FastObjectDump);
  bSizer11->Add(FastObjDump, 0, wxALL, 5);


  m_panel6->SetSizer(bSizer11);
  m_panel6->Layout();
  bSizer11->Fit(m_panel6);
  bSizer10->Add(m_panel6, 0, wxALL, 5);


  bSizer1->Add(bSizer10, 1, wxEXPAND, 5);

  wxStaticLine* m_staticline1;
  m_staticline1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
  bSizer1->Add(m_staticline1, 0, wxEXPAND | wxALL, 0);

  wxPanel* m_panel9;
  m_panel9 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  wxBoxSizer* bSizer14;
  bSizer14 = new wxBoxSizer(wxHORIZONTAL);

  wxPanel* m_panel10;
  m_panel10 = new wxPanel(m_panel9, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  bSizer14->Add(m_panel10, 1, wxEXPAND | wxALL, 5);

  ApplyButton = new wxButton(m_panel9, wxID_OK, wxT("Apply"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer14->Add(ApplyButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  CancelButton = new wxButton(m_panel9, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
  bSizer14->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


  m_panel9->SetSizer(bSizer14);
  m_panel9->Layout();
  bSizer14->Fit(m_panel9);
  bSizer1->Add(m_panel9, 1, wxALL | wxEXPAND, 5);


  this->SetSizer(bSizer1);
  this->Layout();

  this->Centre(wxBOTH);

  UseBuiltInS1Game32->SetValue(CurrentConfig.UseBuiltInS1Game32);
  UseBuiltInS1Game32->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(SettingsWindow::OnUseBuiltInS1Game32), NULL, this);

  if (!IsValidDir(PathField->GetValue()) && !UseBuiltInS1Game32->GetValue())
  {
    ApplyButton->Enable(false);
    ApplyButton->SetToolTip(wxT("You must specify the \"" + RootDir + "\". Press \"Browse\" and select the folder."));
  }
  if (!AllowRebuild || !IsValidDir(CurrentConfig.RootDir.WString()))
  {
    RebuildCacheButton->Enable(false);
    UpdateMappingButton->Enable(false);
  }
}

void SettingsWindow::OpenUpdateLink()
{
  wxLaunchDefaultBrowser(UpdatesUrl);
}

void SettingsWindow::OnBrowseClicked(wxCommandEvent&)
{
  wxDirDialog* openPanel = new wxDirDialog(this, wxS("Select \"") + RootDir + wxS("\" folder"), PathField->GetLabelText());
  openPanel->Center();
  if (openPanel->ShowModal() == wxID_OK)
  {
    const wxString path = openPanel->GetPath();
    if (IsValidDir(path))
    {
      PathField->SetLabelText(path);
    }
    else
    {
      REDialog::Error(wxS("Folder must be called \"") + RootDir + wxS("\""), "Invalid folder name!");
    }
  }
  openPanel->Destroy();
}

void SettingsWindow::OnPathChanged(wxCommandEvent&)
{
  if (!IsValidDir(PathField->GetValue()))
  {
    ApplyButton->Enable(UseBuiltInS1Game32->GetValue());
    ApplyButton->SetToolTip(wxT("You must specify the \"" + RootDir + "\". Press \"Browse\" and select the folder."));
  }
  else
  {
    ApplyButton->Enable(true);
    NewConfig.RootDir = PathField->GetValue().ToStdWstring();
  }
}

void SettingsWindow::OnUpdateDirCacheClicked(wxCommandEvent&)
{
  ProgressWindow progress(this);
  progress.SetCurrentProgress(-1);
  progress.SetCanCancel(false);
  bool ok = true;
  std::thread([&progress, &ok] {
    SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating folder cache..."));
    Sleep(200);
    try
    {
      FPackage::UpdateDirCache();
    }
    catch (const std::exception& e)
    {
      REDialog::Error(e.what());
      ok = false;
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();
  progress.ShowModal();
  if (ok)
  {
    REDialog::Info("Folder cache has been updated!");
  }
}

void SettingsWindow::OnUpdateMappersClicked(wxCommandEvent&)
{
  ProgressWindow progress(this);
  progress.SetCurrentProgress(-1);
  bool ok = true;
  std::thread([&progress, &ok] {
    SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating package mapper..."));
    Sleep(200);
    try
    {
      FPackage::LoadPkgMapper(true);
    }
    catch (const std::exception& e)
    {
      REDialog::Error(e.what());
      ok = false;
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    if (progress.IsCanceled())
    {
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating composite mapper..."));

    try
    {
      FPackage::LoadCompositePackageMapper(true);
    }
    catch (const std::exception& e)
    {
      REDialog::Error(e.what());
      ok = false;
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    if (progress.IsCanceled())
    {
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating object redirector mapper..."));
    try
    {
      FPackage::LoadObjectRedirectorMapper(true);
    }
    catch (const std::exception& e)
    {
      REDialog::Error(e.what());
      ok = false;
      SendEvent(&progress, UPDATE_PROGRESS_FINISH);
      return;
    }
    SendEvent(&progress, UPDATE_PROGRESS_FINISH);
  }).detach();
  progress.ShowModal();
  if (ok)
  {
    REDialog::Info("Mappers have been updated!");
  }
}

void SettingsWindow::OnResetWarningClicked(wxCommandEvent&)
{
  FAppConfig defaults;

  NewConfig.CompositeDumpPath = FString();
  NewConfig.SavePackageDontShowAgain = defaults.SavePackageDontShowAgain;
  NewConfig.SavePackageOpen = defaults.SavePackageOpen;
  NewConfig.SavePackageOpenDontAskAgain = defaults.SavePackageOpenDontAskAgain;
  NewConfig.LastFilePackages.clear();
  REDialog::Info("Close this windows and press Apply to save changes.", "UI settings were cleared!");
}

void SettingsWindow::OnRegisterClicked(wxCommandEvent&)
{
  App* app = (App*)wxTheApp;
  wxCommandEvent tmp;
  app->OnRegisterMime(tmp);
  bool registered = app->CheckMimeTypes(true);
  RegisterButton->Enable(!registered);
  UnregisterButton->Enable(registered);
  if (registered)
  {
    REDialog::Info("The file type has been associated successfully!");
  }
  else
  {
    REDialog::Error("Try to start RE as an administrator.", "Failed to associate!");
  }
}

void SettingsWindow::OnUnregisterClicked(wxCommandEvent&)
{
  App* app = (App*)wxTheApp;
  wxCommandEvent tmp;
  app->OnUnregisterMime(tmp);
  bool registered = app->CheckMimeTypes(true);
  RegisterButton->Enable(!registered);
  UnregisterButton->Enable(registered);
  if (!registered)
  {
    REDialog::Info("The file type has been dissociated successfully!");
  }
  else
  {
    REDialog::Error("Try to start RE as an administrator.", "Failed to dissociate!");
  }
}

void SettingsWindow::OnDcToolClicked(wxCommandEvent&)
{
  if (NewConfig.RootDir.Empty())
  {
    REDialog::Error("Set the S1Game folder first!");
    return;
  }
  DcToolDialog dlg(this);
  dlg.ShowModal();
}

void SettingsWindow::OnUpdateClicked(wxCommandEvent&)
{
  SettingsWindow::OpenUpdateLink();
}

void SettingsWindow::OnCancelClicked(wxCommandEvent&)
{
  EndModal(wxID_CANCEL);
}

void SettingsWindow::OnOkClicked(wxCommandEvent&)
{
  NewConfig.FastObjectDump = FastObjDump->GetValue();
  std::wstring path = NewConfig.UseBuiltInS1Game32 ? NewConfig.RootDir.WString() : PathField->GetValue().ToStdWstring();
  FPackage::S1DirError err = FPackage::ValidateRootDirCandidate(path);
  if (err == FPackage::S1DirError::OK)
  {
    EndModal(wxID_OK);
  }
  else
  {
    if (err == FPackage::S1DirError::NOT_FOUND)
    {
      REDialog::Error("S1Game folder does not exist or can't be accessed.");
    }
    else if (err == FPackage::S1DirError::NAME_MISSMATCH)
    {
      REDialog::Error("The folder must be called S1Game!");
    }
    else if (err == FPackage::S1DirError::CLASSES_NOT_FOUND)
    {
      REDialog::Error("S1Game folder does not contain neccessery *.U files.");
    }
    else if (err == FPackage::S1DirError::ACCESS_DENIED && CurrentConfig.RootDir != PathField->GetValue().ToStdWstring())
    {
      if (REDialog::Auth())
      {
        App::GetSharedApp()->GetConfig() = NewConfig;
        App::GetSharedApp()->RestartElevated();
      }
    }
    else
    {
      EndModal(wxID_OK);
    }
  }
}

void SettingsWindow::OnUseBuiltInS1Game32(wxCommandEvent&)
{
  NewConfig.UseBuiltInS1Game32 = UseBuiltInS1Game32->GetValue();
  PathField->Enable(!NewConfig.UseBuiltInS1Game32);
  BrowseButton->Enable(!NewConfig.UseBuiltInS1Game32);
  if (NewConfig.UseBuiltInS1Game32)
  {
    NewConfig.TempS1GameDir = NewConfig.RootDir;
    PathField->SetValue("N/A");
    NewConfig.RootDir = GetS1Game32Path().ToStdWstring();
  }
  else
  {
    NewConfig.RootDir = NewConfig.TempS1GameDir;
    PathField->SetValue(NewConfig.RootDir.WString());
  }
}

wxBEGIN_EVENT_TABLE(SettingsWindow, WXDialog)
EVT_BUTTON(ControlElementId::Browse, SettingsWindow::OnBrowseClicked)
EVT_BUTTON(wxID_OK, SettingsWindow::OnOkClicked)
EVT_BUTTON(wxID_CANCEL, SettingsWindow::OnCancelClicked)
EVT_BUTTON(ControlElementId::Register, SettingsWindow::OnRegisterClicked)
EVT_BUTTON(ControlElementId::Unregister, SettingsWindow::OnUnregisterClicked)
EVT_BUTTON(ControlElementId::Update, SettingsWindow::OnUpdateClicked)
EVT_BUTTON(ControlElementId::DcTool, SettingsWindow::OnDcToolClicked)
EVT_BUTTON(ControlElementId::DirCache, SettingsWindow::OnUpdateDirCacheClicked)
EVT_BUTTON(ControlElementId::Mappers, SettingsWindow::OnUpdateMappersClicked)
EVT_BUTTON(ControlElementId::Warnings, SettingsWindow::OnResetWarningClicked)
EVT_TEXT(ControlElementId::Path, SettingsWindow::OnPathChanged)
EVT_TEXT_ENTER(ControlElementId::Path, SettingsWindow::OnOkClicked)
wxEND_EVENT_TABLE()
