#pragma once
#include <wx/wx.h>
class RootPathWindow : public wxDialog
{
public:
  RootPathWindow();
  wxString GetRootPath() const;

private:
  void OnPathChanged(wxCommandEvent& e);
  void OnSelectClicked(wxCommandEvent& e);
  void OnContinueClicked(wxCommandEvent& e);

  wxDECLARE_EVENT_TABLE();

private:
  wxTextCtrl* PathTextfield = nullptr;
  wxButton* SelectButton = nullptr;
  wxButton* ContinueButton = nullptr;
  wxString S1CookedPC;
};

