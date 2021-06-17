#pragma once
#include <wx/wx.h>
#include <wx/dataview.h>
#include <wx/statusbr.h>

#include "../Misc/wxModalWindow.h"

class WelcomeDialog : public wxModalWindow {
public:
  WelcomeDialog(wxWindow* parent, bool startMode = false);
  ~WelcomeDialog();

  void OnExternalOpen(const wxString& path);

  std::vector<wxString> GetOpenList() const
  {
    return QueuedOpenList;
  }

  int ShowModal() override;

protected:
  void OnRecentActivated(wxDataViewEvent& event);
  void OnRecentSelected(wxDataViewEvent& event);
  void OnOpenFileClicked(wxCommandEvent& event);
  void OnOpenByNameText(wxCommandEvent& event);
  void OnOpenByNameEnter(wxCommandEvent& event);
  void OnOpenByNameClicked(wxCommandEvent& event);
  void OnObjectDumpClicked(wxCommandEvent& event);
  void OnDataCenterClicked(wxCommandEvent& event);
  void OnSettingsClicked(wxCommandEvent& event);
  void OnUpdateClicked(wxCommandEvent& event);
  void OnCloseClicked(wxCommandEvent& event);
  void OnCloseWinClicked(wxCloseEvent& event);

protected:
  wxDataViewCtrl* RecentCtrl = nullptr;
  wxButton* OpenFileButton = nullptr;
  wxTextCtrl* OpenByNameField = nullptr;
  wxButton* OpenByNameButton = nullptr;
  wxButton* ObjectDumpButton = nullptr;
  wxButton* DataCenterButton = nullptr;
  wxButton* SettingsButton = nullptr;
  wxButton* UpdateButton = nullptr;
  wxStaticText* Version = nullptr;
  wxButton* CloseButton = nullptr;
  wxStatusBar* StatusBar = nullptr;

  bool StartMode = false;
  bool ModalRunning = false;

  std::vector<wxString> QueuedOpenList;
};