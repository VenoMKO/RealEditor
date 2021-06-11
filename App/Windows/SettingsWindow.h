#pragma once
#include <wx/wx.h>
#include <Utils/AConfiguration.h>

class SettingsWindow : public wxDialog {
public:
  SettingsWindow(const FAppConfig& currentConfig, FAppConfig& output, bool allowRebuild, const wxPoint& pos = wxDefaultPosition);

private:
  void OnBrowseClicked(wxCommandEvent&);
  void OnPathChanged(wxCommandEvent&);

  void OnUpdateDirCacheClicked(wxCommandEvent&);
  void OnUpdateMappersClicked(wxCommandEvent&);
  void OnResetWarningClicked(wxCommandEvent&);

  void OnRegisterClicked(wxCommandEvent&);
  void OnUnregisterClicked(wxCommandEvent&);

  void OnDcToolClicked(wxCommandEvent&);
  void OnUpdateClicked(wxCommandEvent&);

  void OnCancelClicked(wxCommandEvent&);
  void OnOkClicked(wxCommandEvent&);

private:
  FAppConfig CurrentConfig;
  FAppConfig& NewConfig;

  wxTextCtrl* PathField;
  wxButton* BrowseButton;
  wxButton* RebuildCacheButton;
  wxButton* UpdateMappingButton;
  wxButton* ResetWarningsButton;
  wxButton* RegisterButton;
  wxButton* UnregisterButton;
  wxButton* UpdatesButton;
  wxStaticText* VersionLabel;
  wxButton* CancelButton;
  wxButton* ApplyButton;

  bool AllowRebuild = true;
  bool WasRegistered = false;

  wxDECLARE_EVENT_TABLE();
};